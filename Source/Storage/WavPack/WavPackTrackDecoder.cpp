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

#include "./WavPackReader.h"

#include <cassert> // for assert()
#include <limits> // for std::numeric_limits

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  WavPackTrackDecoder::WavPackTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    reader(file),
    channelOrder(),
    totalFrameCount(0),
    nativeSampleFormat(AudioSampleFormat::Unknown),
    decodingMutex() {

    this->totalFrameCount = this->reader.CountTotalFrames();
    this->channelOrder = this->reader.GetChannelOrder();
    this->nativeSampleFormat = this->reader.GetSampleFormat();
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> WavPackTrackDecoder::Clone() const {
    throw std::runtime_error(u8"Not implemented yet");
    //return std::make_shared<WavPackTrackDecoder>(this->state->File);
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
    return this->totalFrameCount;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackTrackDecoder::GetNativeSampleFormat() const {
    return this->nativeSampleFormat;
  }

  // ------------------------------------------------------------------------------------------- //

  bool WavPackTrackDecoder::IsNativelyInterleaved() const {
    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackTrackDecoder::DecodeInterleavedUint8(
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

  void WavPackTrackDecoder::DecodeInterleavedInt16(
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

  void WavPackTrackDecoder::DecodeInterleavedInt32(
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

  void WavPackTrackDecoder::DecodeInterleavedFloat(
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

  void WavPackTrackDecoder::DecodeInterleavedDouble(
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

  void WavPackTrackDecoder::DecodeSeparatedUint8(
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

  void WavPackTrackDecoder::DecodeSeparatedInt16(
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

  void WavPackTrackDecoder::DecodeSeparatedInt32(
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

  void WavPackTrackDecoder::DecodeSeparatedFloat(
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

  void WavPackTrackDecoder::DecodeSeparatedDouble(
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

  void WavPackTrackDecoder::verifyDecodeRange(
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

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
