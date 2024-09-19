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
#include "Nuclex/Audio/TrackInfo.h"

#include "../Shared/ChannelOrderFactory.h"
#include "./FlacReader.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper that copies and interleaves samples returne by libflac</summary>
  class InterleavingSampleForwarder {

    public: InterleavingSampleForwarder(
      float *target, std::size_t channelCount, std::size_t bitsPerSample
    ) :
      target(target),
      channelCount(channelCount),
      factor(0.0f) {

      this->factor = static_cast<float>(
        1.0 / static_cast<double>((1 << (bitsPerSample - 1)) - 1)
      );
    }

    /// <summary>Writes the decoded samples into the user-provided buffer</summary>
    /// <param name="buffers">
    ///   Buffers (allocated and provided by libflac) containing the separated audio channels
    /// </parma>
    /// <param name="frameCount">
    ///   Total number of frames (= samples in each channel) delivered
    /// </param>
    public: void WriteDecodedSamples(
      const std::int32_t *const buffers[], std::size_t frameCount
    ) {
      for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          *this->target = static_cast<float>(buffers[channelIndex][frameIndex]) * this->factor;
          ++this->target;
        }
      }
    }

    /// <summary>
    ///   Static callback sink that forwards to the WriteDecoddSamples() method
    /// </summary>
    /// <param name="userPointer">The instance of this class to forward to</param>
    /// <param name="buffers">
    ///   Buffers (allocated and provided by libflac) containing the separated audio channels
    /// </parma>
    /// <param name="frameCount">
    ///   Total number of frames (= samples in each channel) delivered
    /// </param>
    public: static void ProcessDecodedSamplesFunction(
      void *userPointer, const std::int32_t *const buffers[], std::size_t frameCount
    ) {
      reinterpret_cast<InterleavingSampleForwarder *>(userPointer)->WriteDecodedSamples(
        buffers, frameCount
      );
    }

    private: float *target;
    private: std::size_t channelCount;
    private: float factor;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  FlacTrackDecoder::FlacTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    reader(file),
    trackInfo(),
    channelOrder(),
    totalSampleCount(std::uint64_t(-1)),
    decodingMutex() {

    this->reader.ReadMetadata(this->trackInfo);

    // Just like Waveform, in WavPack the channel order matches the order of the flag bits.
    this->channelOrder = Shared::ChannelOrderFactory::FromWaveformatExtensibleLayout(
      static_cast<std::size_t>(this->trackInfo.ChannelCount),
      static_cast<ChannelPlacement>(this->trackInfo.ChannelPlacements)
    );

    this->totalSampleCount = this->reader.CountTotalFrames();
  }

  // ------------------------------------------------------------------------------------------- //

  FlacTrackDecoder::~FlacTrackDecoder() {}

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> FlacTrackDecoder::Clone() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t FlacTrackDecoder::CountChannels() const {
    return this->channelOrder.size();
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> &FlacTrackDecoder::GetChannelOrder() const {
    return this->channelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t FlacTrackDecoder::CountFrames() const {
    return this->totalSampleCount;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat FlacTrackDecoder::GetNativeSampleFormat() const {
    return this->trackInfo.SampleFormat;
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
    std::int16_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedInt32(
    std::int32_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedFloat(
    float *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    if(std::numeric_limits<std::uint32_t>::max() < frameCount) {
      throw std::logic_error(u8"Unable to decode this many samples in one call");
    }
    if(startFrame >= this->totalSampleCount) {
      throw std::out_of_range(u8"Start sample index is out of bounds");
    }
    if(this->totalSampleCount < startFrame + frameCount) {
      throw std::out_of_range(u8"Decode sample count goes beyond the end of audio data");
    }

    {
      std::lock_guard<std::mutex> decodingMutexScope(this->decodingMutex);

      // If the caller requests to read from a location that is not where the file cursor
      // is currently at, we need to seek to that position first.
      if(this->reader.GetFrameCursorPosition() != startFrame) {
        this->reader.Seek(startFrame);
      }

      InterleavingSampleForwarder forwarder(
        buffer, this->channelOrder.size(), this->trackInfo.BitsPerSample
      );

      this->reader.DecodeSeparated(
        &forwarder,
        &InterleavingSampleForwarder::ProcessDecodedSamplesFunction,
        frameCount
      );
    } // mutex lock scope
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacTrackDecoder::DecodeInterleavedDouble(
    double *buffer, const std::uint64_t startFrame, const std::size_t frameCount
  ) const {
    (void)buffer;
    (void)startFrame;
    (void)frameCount;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
