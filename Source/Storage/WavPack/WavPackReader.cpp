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

#include "./WavPackReader.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/TrackInfo.h"

#include "./WavPackVirtualFileAdapter.h"
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/WavPackApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackReader::SampleFormatFromModeAndBitsPerSample(
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

  WavPackReader::WavPackReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    streamReader(),
    state(),
    context(),
    mode(0),
    bitsPerSample(0),
    bytesPerSample(0),
    frameCursor(0) {

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

    this->mode = Platform::WavPackApi::GetMode(this->context);
    this->bitsPerSample = Platform::WavPackApi::GetBitsPerSample(context);
    this->bytesPerSample = Platform::WavPackApi::GetBytesPerSample(context);
  }

  // ------------------------------------------------------------------------------------------- //

  WavPackReader::~WavPackReader() {}

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::ReadMetadata(TrackInfo &target) {
    target.ChannelCount = Platform::WavPackApi::GetNumChannels(this->context);

    target.ChannelPlacements = static_cast<ChannelPlacement>(
      Platform::WavPackApi::GetChannelMask(this->context)
    );

    target.SampleFormat = GetSampleFormat();

    target.SampleRate = Platform::WavPackApi::GetSampleRate(this->context);
    target.BitsPerSample = this->bitsPerSample;

    std::uint64_t totalSampleCount = Platform::WavPackApi::GetNumSamples64(this->context);
    target.Duration = std::chrono::microseconds(
      totalSampleCount * 1'000'000 / target.SampleRate
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WavPackReader::CountTotalFrames() const {
    return Platform::WavPackApi::GetNumSamples64(this->context);
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackReader::GetSampleFormat() const {
    return SampleFormatFromModeAndBitsPerSample(this->mode, this->bitsPerSample);
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> WavPackReader::GetChannelOrder() const {

    int wavPackChannelCount = Platform::WavPackApi::GetNumChannels(this->context);
    int wavPackChannelMask = Platform::WavPackApi::GetChannelMask(this->context);

    // Just like Waveform, in WavPack the channel order matches the order of the flag bits.
    return Shared::ChannelOrderFactory::FromWaveformatExtensibleLayout(
      static_cast<std::size_t>(wavPackChannelCount),
      static_cast<ChannelPlacement>(wavPackChannelMask)
    );

  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WavPackReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::Seek(std::uint64_t frameIndex) {

    // ISSUE: SeekSample64() is documented as bringing the context into an invalid state
    // when it returns an error and that no further operations besides closing
    // the WavPack file are supported after that. Should we intervene to guarantee
    // correct behavior or should we leave it up to random chance / the user?
    Platform::WavPackApi::SeekSample(this->state->Error, this->context, frameIndex);
    this->frameCursor = frameIndex;
    //StreamAdapterState::RethrowPotentialException(*state);

  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::DecodeInterleaved(std::int32_t *buffer, std::size_t frameCount) {
    if(std::numeric_limits<std::uint32_t>::max() < frameCount) {
      throw std::invalid_argument(u8"Unable to decode that many samples in a single call");
    }

    std::uint32_t unpackedFrameCount = Platform::WavPackApi::UnpackSamples(
      this->state->Error, // exception_ptr that will receive VirtualFile exceptions
      this->context,
      buffer,
      static_cast<std::uint32_t>(frameCount)
    );

    this->frameCursor += unpackedFrameCount;

    if(unpackedFrameCount != frameCount) {
      throw std::runtime_error(
        u8"libwavpack unpacked a different number of samples than was requested. "
        u8"Truncated file?"
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
