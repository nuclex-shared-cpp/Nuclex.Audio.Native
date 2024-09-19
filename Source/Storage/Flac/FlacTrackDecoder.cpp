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

    /// <summary>Initializes a new interleaving sample forward</summary>
    /// <param name="target">Buffer into which to write the interleaved samples</param>
    /// <param name="channelCount">Number of channels that are being decoded</param>
    /// <param name="bitsPerSample">Number of bits per audio sample</param>
    public: InterleavingSampleForwarder(
      float *target, std::size_t channelCount, std::size_t bitsPerSample
    );

    /// <summary>Writes the decoded samples into the user-provided buffer</summary>
    /// <param name="buffers">
    ///   Buffers (allocated and provided by libflac) containing the separated audio channels
    /// </parma>
    /// <param name="frameCount">
    ///   Total number of frames (= samples in each channel) delivered
    /// </param>
    public: void WriteDecodedSamples(
      const std::int32_t *const buffers[], std::size_t frameCount
    );

    /// <summary>
    ///   Static callback sink that forwards to the WriteDecodedSamples() method
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
    );

    /// <summary>Target buffer that receives the interleaved samples</summary>
    private: float *target;
    /// <summary>Number of audio channels libflac is decoding for us</summary>
    private: std::size_t channelCount;
    /// <summary>Factor by which samples need to be scaled</summary>
    private: float factor;

  };

  // ------------------------------------------------------------------------------------------- //

  InterleavingSampleForwarder::InterleavingSampleForwarder(
    float *target, std::size_t channelCount, std::size_t bitsPerSample
  ) :
    target(target),
    channelCount(channelCount),
    factor(0.0f) {

    this->factor = static_cast<float>(
      1.0 / static_cast<double>((1 << (bitsPerSample - 1)) - 1)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void InterleavingSampleForwarder::WriteDecodedSamples(
    const std::int32_t *const buffers[], std::size_t frameCount
  ) {

    // The audio samples actually do sit in the least significant bits for libflac,
    // tested on x86 with 16-bit asnd 24-bit .flac audio files.
    //
    // Also, they're separated, not interleaved, but our public interface currently
    // only supports interleaved sample output. Does this result in an efficient access
    // pattern? Or would the CPU cache be batter served by doing it channel-by-channel?
    //
    for(std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
      for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
        *this->target = static_cast<float>(buffers[channelIndex][frameIndex]) * this->factor;
        ++this->target;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void InterleavingSampleForwarder::ProcessDecodedSamplesFunction(
    void *userPointer, const std::int32_t *const buffers[], std::size_t frameCount
  ) {
    reinterpret_cast<InterleavingSampleForwarder *>(userPointer)->WriteDecodedSamples(
      buffers, frameCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  FlacTrackDecoder::FlacTrackDecoder(const std::shared_ptr<const VirtualFile> &file) :
    reader(file),
    trackInfo(),
    channelOrder(),
    totalFrameCount(std::uint64_t(-1)),
    decodingMutex() {

    this->reader.ReadMetadata(this->trackInfo);

    // Just like Waveform, in WavPack the channel order matches the order of the flag bits.
    this->channelOrder = Shared::ChannelOrderFactory::FromWaveformatExtensibleLayout(
      static_cast<std::size_t>(this->trackInfo.ChannelCount),
      static_cast<ChannelPlacement>(this->trackInfo.ChannelPlacements)
    );

    this->totalFrameCount = this->reader.CountTotalFrames();
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
    return this->totalFrameCount;
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
    if(startFrame >= this->totalFrameCount) {
      throw std::out_of_range(u8"Start sample index is out of bounds");
    }
    if(this->totalFrameCount < startFrame + frameCount) {
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
