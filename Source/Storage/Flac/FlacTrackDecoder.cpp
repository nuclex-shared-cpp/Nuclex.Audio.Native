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

#include "./FlacTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/Processing/SampleConverter.h"

#include "./FlacReader.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  FlacTrackDecoder::FlacTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    reader(std::make_unique<FlacReader>(file)),
    channelOrder(),
    totalSampleCount(std::uint64_t(-1)),
    sampleFormat(AudioSampleFormat::Unknown),
    bitsPerSample(0),
    sampleCursor(0),
    decodingMutex() {}

  // ------------------------------------------------------------------------------------------- //

  FlacTrackDecoder::~FlacTrackDecoder() {}

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> FlacTrackDecoder::Clone() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t FlacTrackDecoder::CountChannels() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> &FlacTrackDecoder::GetChannelOrder() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t FlacTrackDecoder::CountFrames() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat FlacTrackDecoder::GetNativeSampleFormat() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  bool FlacTrackDecoder::IsNativelyInterleaved() const {
    return false; // FLAC actually separates the audio channels
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedUint8(
    std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedInt16(
    std::int16_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    (void)buffer;
    (void)startSample;
    (void)sampleCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedInt32(
    std::int32_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    (void)buffer;
    (void)startSample;
    (void)sampleCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedFloat(
    float *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    (void)buffer;
    (void)startSample;
    (void)sampleCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    (void)buffer;
    (void)startSample;
    (void)sampleCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::fetchChannelOrder() {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
