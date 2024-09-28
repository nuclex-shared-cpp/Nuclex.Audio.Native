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

#include "./OpusTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/Processing/SampleConverter.h"
#include "Nuclex/Audio/TrackInfo.h"

#include "./OpusReader.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  OpusTrackDecoder::OpusTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
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
  }

  // ------------------------------------------------------------------------------------------- //

  OpusTrackDecoder::~OpusTrackDecoder() {}

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> OpusTrackDecoder::Clone() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t OpusTrackDecoder::CountChannels() const {
    return this->channelOrder.size();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> &OpusTrackDecoder::GetChannelOrder() const {
    return this->channelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t OpusTrackDecoder::CountFrames() const {
    return this->totalFrameCount;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat OpusTrackDecoder::GetNativeSampleFormat() const {
    return this->trackInfo.SampleFormat;
  }

  // ------------------------------------------------------------------------------------------- //

  bool OpusTrackDecoder::IsNativelyInterleaved() const {
    return true; // All exposed Opus read methods provide interleaved samples
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeInterleavedUint8(
    std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeInterleaved(buffer, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeInterleavedInt16(
    std::int16_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeInterleaved(buffer, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeInterleavedInt32(
    std::int32_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeInterleaved(buffer, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeInterleavedFloat(
    float *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeInterleaved(buffer, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeInterleaved(buffer, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeSeparatedUint8(
    std::uint8_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeSeparated(buffers, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeSeparatedInt16(
    std::int16_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeSeparated(buffers, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeSeparatedInt32(
    std::int32_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeSeparated(buffers, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeSeparatedFloat(
    float *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeSeparated(buffers, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::DecodeSeparatedDouble(
    double *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    verifyDecodeRange(startFrame, frameCount);

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      this->reader.DecodeSeparated(buffers, frameCount);

    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackDecoder::verifyDecodeRange(
    const std::uint64_t startFrame, const std::size_t frameCount
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
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
