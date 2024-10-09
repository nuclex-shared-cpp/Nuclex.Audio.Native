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

#include "./WaveformTrackDecoder.h"

#include "Nuclex/Audio/Processing/SampleConverter.h"
#include "Nuclex/Audio/TrackInfo.h"

#include "./WaveformReader.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  WaveformTrackDecoder::WaveformTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    reader(file),
    trackInfo(),
    channelOrder(),
    totalFrameCount(std::uint64_t(-1)),
    decodingMutex() {

    this->reader.ReadMetadata(this->trackInfo);

    // Opus uses the same custom channel order as Vorbis, which the OpusReader will
    // correctly determine for us here
    this->channelOrder = this->reader.GetChannelOrder();

    this->totalFrameCount = this->reader.CountTotalFrames();

    this->reader.PrepareForReading();
  }

  // ------------------------------------------------------------------------------------------- //

  WaveformTrackDecoder::~WaveformTrackDecoder() {}

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> WaveformTrackDecoder::Clone() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WaveformTrackDecoder::CountChannels() const {
    return this->channelOrder.size();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> & WaveformTrackDecoder::GetChannelOrder() const {
    return this->channelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WaveformTrackDecoder::CountFrames() const {
    return this->totalFrameCount;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WaveformTrackDecoder::GetNativeSampleFormat() const {
    return this->trackInfo.SampleFormat;
  }

  // ------------------------------------------------------------------------------------------- //

  bool WaveformTrackDecoder::IsNativelyInterleaved() const {
    return true; // Waveform stores the data uncompressed and with interleaved channels
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeInterleavedUint8(
    std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeInterleavedInt16(
    std::int16_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeInterleavedInt32(
    std::int32_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeInterleavedFloat(
    float *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    if(std::numeric_limits<std::uint32_t>::max() < frameCount) {
      throw std::logic_error(u8"Unable to decode this many samples in one call");
    }
    if(startFrame >= this->totalFrameCount) {
      throw std::out_of_range(u8"Start sample index is out of bounds");
    }
    if(this->totalFrameCount < startFrame + frameCount) {
      throw std::out_of_range(u8"Decode sample count goes beyond the end of audio data");
    }

    this->reader.ReadInterleaved<float>(buffer, startFrame, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeSeparatedUint8(
    std::uint8_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffers;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeSeparatedInt16(
    std::int16_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffers;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeSeparatedInt32(
    std::int32_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffers;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeSeparatedFloat(
    float *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffers;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformTrackDecoder::DecodeSeparatedDouble(
    double *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffers;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
