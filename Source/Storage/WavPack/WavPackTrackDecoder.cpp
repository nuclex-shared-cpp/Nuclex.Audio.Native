#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "./WavPackTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  // TODO: Duplicate code! Abstract virtual file adapter concept and put this there.

  /// <summary>Checks a virtual file / WavPack stream adapter for exceptions</summary>
  /// <remarks>
  ///   <para>
  ///     We can't throw exceptions right through libwavpack's C code (well, we could,
  ///     but it would interrupt execution at points where libwavpack doesn't expect it),
  ///     so the virtual file adapter records exceptions that come out of the virtual file
  ///     being accessed.
  ///   </para>
  ///   <para>
  ///     This small helper class will check if there are any exceptions recorded in
  ///     a virtual file adapter and re-throw them. It is used by the WavPackApi methods
  ///     to re-throw the root cause exception rather than the generic libwavpack error
  ///     if the original failure occurred in the virtual file class.
  ///   </para>
  /// </remarks>
  class ExceptionChecker {

    /// <summary>Initializes a new exception checker for the specified adapter state</summary>
    /// <param name="state">Adapter state that will be checked</param>
    public: ExceptionChecker(Nuclex::Audio::Storage::WavPack::StreamAdapterState &state) :
      state(state) {}

    /// <summary>
    ///   Checks whether any exceptions have occurred in the stream adapter and, if so,
    ///   re-throws them
    /// </summary>
    public: void Check() {
      Nuclex::Audio::Storage::WavPack::StreamAdapterState::RethrowPotentialException(
        this->state
      );
    }

    /// <summary>The stream adapter from which exceptions will be re-thrown</summary>
    private: Nuclex::Audio::Storage::WavPack::StreamAdapterState &state;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackTrackDecoder::SampleFormatFromModeAndBitsPerSample(
    int mode, int bitsPerSample
  ) {

    // Figure out the data format closest to the data stored by WavPack. Normally it
    // should be an exact match, but WavPack leaves room to store fewer bits, not only
    // for 24-bit formats. For the sake of robustness, we'll anticipate those, too.
    if((mode & MODE_FLOAT) != 0) {
      if(bitsPerSample >= 33) {
        return Nuclex::Audio::AudioSampleFormat::Float_64;
      } else {
        return Nuclex::Audio::AudioSampleFormat::Float_32;
      }
    } else {
      if(bitsPerSample >= 25) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
      } else if(bitsPerSample >= 17) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
      } else if(bitsPerSample >= 9) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
      } else {
        return Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  WavPackTrackDecoder::WavPackTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    streamReader(),
    state(),
    context(),
    channelOrder() {

    // Set up a WavPack stream reader with adapter methods that will perform all reads
    // on the provided virtual file.
    this->state = std::move(
      StreamAdapterFactory::CreateAdapterForReading(file, this->streamReader)
    );

    // Open the WavPack file, obtaining a WavPack context. Everything inside
    // this scope is just error plumbing code, ensuring that the right exception
    // surfaces if either libwavpack reports an error or the virtual file throws.
    {
      using Nuclex::Audio::Platform::WavPackApi;

      ExceptionChecker exceptionChecker(*state);
      this->context = WavPackApi::OpenStreamReaderInput(
        this->state->Error, // exception_ptr that will receive VirtualFile exceptions
        this->streamReader,
        this->state.get() // passed to all IO callbacks as void pointer
      );

      // The OpenStreamReaderInput() method will already have checked for errors,
      // but if some file access error happened that libwavpack deemed non-fatal,
      // we still want to throw it - an exception in VirtualFile should always surface.
      StreamAdapterState::RethrowPotentialException(*state);
    }

    fetchChannelOrder();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> WavPackTrackDecoder::Clone() const {
    return std::make_shared<WavPackTrackDecoder>(this->state->File);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WavPackTrackDecoder::CountChannels() const {
    return this->channelOrder.size();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> &WavPackTrackDecoder::GetChannelOrder() const {
    return this->channelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WavPackTrackDecoder::CountFrames() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackTrackDecoder::GetNativeSampleFormat() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::fetchChannelOrder() {
    int wavPackChannelCount = Platform::WavPackApi::GetNumChannels(this->context);
    int wavPackChannelMask = Platform::WavPackApi::GetChannelMask(this->context);

    // First, add all channels for which a channel flag bit is set. Just like Waveform,
    // in WavPack the channel order matches the order of the flag bits.
    for(std::size_t bitIndex = 0; bitIndex < 17; ++bitIndex) {
      if((static_cast<std::size_t>(wavPackChannelMask) & (1ULL << bitIndex)) != 0) {
        this->channelOrder.push_back(static_cast<ChannelPlacement>(1ULL << bitIndex));
        --wavPackChannelCount;
      }
    }

    // However, in WAVEFORMATEXTENSIBLE (and therefore in WavPack?) it is valid to set
    // the channel mask flags to zero and include channels. These are then arbitrary,
    // non-placeable channels not associated with specific speakers. In such a case,
    // or if the channel count exceeds the number of channel mask bits set, we add
    // the remaining channels as unknown channels.
    while(wavPackChannelCount >= 1) {
      this->channelOrder.push_back(ChannelPlacement::Unknown);
      --wavPackChannelCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedUint8(
    std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedInt16(
    std::int16_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedInt32(
    std::int32_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedFloat(
    float *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
