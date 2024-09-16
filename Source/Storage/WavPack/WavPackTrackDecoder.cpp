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

#include "Nuclex/Audio/Processing/SampleConverter.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Special value found in the bitsPerSample attribute when using floating point samples
  /// </summary>
  constexpr std::size_t FloatBitsPerSample(-32);

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
    channelOrder(),
    bitsPerSample(0),
    totalSampleCount(0),
    sampleCursor(0),
    decodingMutex() {

    // Set up a WavPack stream reader with adapter methods that will perform all reads
    // on the provided virtual file.
    this->state = std::move(
      StreamAdapterFactory::CreateAdapterForReading(file, this->streamReader)
    );

    // Open the WavPack file, obtaining a WavPack context.The exception_ptr is checked
    // inside that WavPackApi wrapper, ensuring that the right exception surfaces if
    // either libwavpack reports an error or the virtual file throws.
    this->context = Platform::WavPackApi::OpenStreamReaderInput(
      this->state->Error, // exception_ptr that will receive VirtualFile exceptions
      this->streamReader,
      this->state.get() // passed to all IO callbacks as void pointer
    );

    // The OpenStreamReaderInput() method will already have checked for errors,
    // but if some file access error happened that libwavpack deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    StreamAdapterState::RethrowPotentialException(*state);

    // Determine the number of bits per sample and whether the WavPack file contains
    // floating point audio samples.
    {
      int mode = Platform::WavPackApi::GetMode(context);
      this->bitsPerSample = Platform::WavPackApi::GetBitsPerSample(context);
      this->sampleFormat = SampleFormatFromModeAndBitsPerSample(mode, this->bitsPerSample);
    }

    this->totalSampleCount = Platform::WavPackApi::GetNumSamples64(this->context);

    // Finally, figure out the order in which channels will be interleaved when decoding
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
    return this->totalSampleCount;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackTrackDecoder::GetNativeSampleFormat() const {
    return this->sampleFormat;
  }

  // ------------------------------------------------------------------------------------------- //

  bool WavPackTrackDecoder::IsNativelyInterleaved() const {
    return true;
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
    if(sampleCount > std::numeric_limits<std::uint32_t>::max()) {
      throw std::logic_error(u8"Unable to unpack this many samples in one call");
    }
    if(startSample >= this->totalSampleCount) {
      throw std::out_of_range(u8"Start sample index is out of bounds");
    }
    if(this->totalSampleCount < startSample + sampleCount) {
      throw std::out_of_range(u8"Decode sample count goes beyond the end of audio data");
    }

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->sampleCursor != startSample) {
        Platform::WavPackApi::SeekSample( 
          this->state->Error, // exception_ptr that will receive VirtualFile exceptions
          this->context,
          startSample // accepted uint64 -> int64 mismatch
        );

        // SeekSample64() is documented as bringing the context into an invalid state
        // when it returns an error and that no further operations besides closing
        // the WavPack file are supported after that. Should we intervene to guarantee
        // correct behavior or should we leave it up to random chance / the user?

        this->sampleCursor = startSample;
      }

      // If the audio data is already using floating point, pick the fast path
      if(this->sampleFormat == AudioSampleFormat::Float_32) {
        std::uint32_t unpackedSampleCount = Platform::WavPackApi::UnpackSamples(
          this->state->Error, // exception_ptr that will receive VirtualFile exceptions
          this->context, reinterpret_cast<std::int32_t *>(buffer), sampleCount
        );
        this->sampleCursor += unpackedSampleCount;

        if(unpackedSampleCount != sampleCount) {
          throw std::runtime_error(
            u8"libwavpack unpacked a different number of samples than was requested. "
            u8"Truncated file?"
          );
        }
      } else if(this->sampleFormat == AudioSampleFormat::SignedInteger_24) {
        std::vector<std::int32_t> samples24(sampleCount * this->channelOrder.size());
        std::uint32_t unpackedSampleCount = Platform::WavPackApi::UnpackSamples(
          this->state->Error, // exception_ptr that will receive VirtualFile exceptions
          this->context, samples24.data(), sampleCount
        );
        this->sampleCursor += unpackedSampleCount;

        if(unpackedSampleCount != sampleCount) {
          throw std::runtime_error(
            u8"libwavpack unpacked a different number of samples than was requested. "
            u8"Truncated file?"
          );
        }

        Processing::SampleConverter::Reconstruct(
          samples24.data(), 24, buffer, sampleCount * this->channelOrder.size()
        );

      } else {
        throw std::runtime_error(u8"Formats other than float not implemented yet");
      }

    } // mutex lock scope

    // TODO: Change SampleConverter or write separate LitteEndianSampleConverter
    // Or not? Does libwavpack talk about LSB-justified or about memory-left-justified?

    // WavPack API documentation for GetBitsPerSample():
    //
    //     "...That is, values are right justified when unpacked into ints, but are
    //     left justified in the number of bytes used by the original data.'
    //
    // This is currently not the exact behavior of the SampleConverter, which always
    // fills the most significant bits with the samples, even if bytes remain empty.
    //
    // It will bite us with 24-bit audio samples stored in 32-bit integers.
    //
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);
    throw std::runtime_error(u8"Formats other than float not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
