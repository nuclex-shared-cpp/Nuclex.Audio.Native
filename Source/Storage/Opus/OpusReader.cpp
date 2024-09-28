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

#include "./OpusReader.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "./OpusVirtualFileAdapter.h" // for FileAdapterFactory
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/OpusApi.h" // for OpusApi

#include "Nuclex/Audio/TrackInfo.h"
#include "Nuclex/Audio/Errors/CorruptedFileError.h"
#include "Nuclex/Audio/Processing/SampleConverter.h"

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <cmath>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes audio samples from an opened Opus file and converts them</summary>
  /// <typeparam name="TTargetSample">
  ///   Data type in which the target samples will be stored
  /// </typeparam>
  /// <param name="opusFile">Opened Opus file samples will be decoded from</param>
  /// <param name="channelCount">Number of channels in the audio file</param>
  /// <param name="frameCursor">Current decoding cursor in the Opus file</param>
  /// <param name="target">Buffer that will receive the converted samples</param>
  /// <param name="frameCount">Number of frames that will be decoded</param>
  template<typename TTargetSample>
  void decodeOpusAndConvert(
    const std::shared_ptr<::OggOpusFile> &opusFile,
    std::size_t channelCount,
    std::uint64_t &frameCursor,
    TTargetSample *target,
    std::size_t frameCount
  ) {
    std::vector<float> intermediateBuffer;
    intermediateBuffer.resize(1024 * channelCount);

    while(frameCount >= 1) {

      // Decode into a float buffer which we will later convert into the target type.
      // We always use the float decoding method, even for int16, because the int16
      // decoding method in libopusfile does dithering. That's nice for playback,
      // but for this library's output, we don't want one audio format to dither and
      // another to not dither.
      std::size_t decodedFrameCount = Nuclex::Audio::Platform::OpusApi::ReadFloat(
        opusFile,
        intermediateBuffer.data(),
        static_cast<int>(intermediateBuffer.size() * channelCount)
      );
      if(decodedFrameCount == 0) {
        throw Nuclex::Audio::Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Opus file. File truncated?"
        );
      }

      // Decoding was done, the frame cursor inside libopusfile has moved, so update
      // our frame cursor as well. Even if something goes wrong while handling the samples,
      // it's important we track where libopusfile currently is so we don't decode
      // the wrong samples next.
      frameCursor += decodedFrameCount;

      // Either convert the decoded floats to doubles or quantize them into integers,
      // depending on the target type. We could just leave it up to SampleConverter::Convert(),
      // but at this point, we know the correct operation at compile time.
      if constexpr(std::is_same<TTargetSample, double>::value) {
        Nuclex::Audio::Processing::SampleConverter::ExtendBits(
          intermediateBuffer.data(), 32,
          target, 64,
          decodedFrameCount * channelCount
        );
      } else {
        Nuclex::Audio::Processing::SampleConverter::Quantize(
          intermediateBuffer.data(),
          target, 32,
          decodedFrameCount * channelCount
        );
      }

      // Buffer was filled by the sample conversion method above, update the buffer pointer
      // to point to where the next batch of samples needs to be written
      target += decodedFrameCount * channelCount;
      if(decodedFrameCount > frameCount) {
        assert((frameCount >= decodedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }

      frameCount -= decodedFrameCount;

    } // while frames left to decode
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  OpusReader::OpusReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    fileCallbacks(),
    state(),
    opusFile(),
    channelCount(0),
    frameCursor(0) {

    // Set up the libopusfile callbacks with adapter methods that will perform all reads
    // on the user-provided virtual file.
    this->state = (
      FileAdapterFactory::CreateAdapterForReading(file, this->fileCallbacks)
    );

    // Open the Opus file, obtaining a OggOpusFile instance. Everything inside
    // this scope is just error plumbing code, ensuring that the right exception
    // surfaces if either libopusfile reports an error or the virtual file throws.
    this->opusFile = Platform::OpusApi::OpenFromCallbacks(
      state->Error,
      state.get(),
      &fileCallbacks
    );

    // The OpenFromCallbacks() method will already have checked for errors,
    // but if some file access error happened that libopusfile deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    FileAdapterState::RethrowPotentialException(*state);

    // Opus audio streams can be chained together (sequentially and not in the sense of
    // interleaving it as another stream in the OGG container). This would mean that
    // the audio stream properties (i.e. channel count, sample rate) might change while
    // we are decoding...
    //
    // TODO: Investigate, in detail, how libopusfile deals with multiple links in Opus files.
    //   I'm unsure of how to deal with this, and the degree to which libopusfile will
    //   automate things - if it just switches to the next link, what if the channel count
    //   suddenly changes? Will libopusfile upmix and downmix? Leave it all to us?
    //
    std::size_t linkCount = Platform::OpusApi::CountLinks(this->opusFile);
    if(linkCount != 1) {
      throw std::runtime_error(u8"Multi-link Opus files are not supported");
    }

    // The channel count is important for the decoding methods
    {
      const ::OpusHead &header = Platform::OpusApi::GetHeader(this->opusFile);
      this->channelCount = header.channel_count;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  OpusReader::~OpusReader() {}

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::ReadMetadata(TrackInfo &target) {
    const ::OpusHead &header = Platform::OpusApi::GetHeader(this->opusFile);

    target.ChannelCount = static_cast<std::size_t>(header.channel_count);

    target.ChannelPlacements = (
      Shared::ChannelOrderFactory::ChannelPlacementFromVorbisFamilyAndCount(
        header.mapping_family, target.ChannelCount
      )
    );

    // Opus audio is always encoded at 48000 samples per second, no matter what the original
    // input sample rate had been. The .input_sample_rate field merely states what
    // the original sample rate had been, but is not useful for playback of the Opus file.
    //trackInfo.SampleRate = static_cast<std::size_t>(header.input_sample_rate)
    target.SampleRate = 48000;

    std::uint64_t totalSampleCount = Platform::OpusApi::CountSamples(opusFile);
    target.Duration = std::chrono::microseconds(totalSampleCount * 1'000 / 48);

    {
      // Completely unfounded, arbitrary value to estimate the precision (which may or may
      // not even change depending on Opus bitrates) of an Opus file compared to any audio
      // format that stores signed integer samples.
      const std::size_t MadeUpOpusPrecisionFromCompressionRatio = 80;

      // Calculate the number of bytes the audio data would decode to
      std::uint64_t decodedByteCount = totalSampleCount * 2; // bytes
      decodedByteCount *= target.ChannelCount;

      target.BitsPerSample = std::max<std::size_t>(
        1, // Let's not report less than 1 bit per sample...
        Platform::OpusApi::GetRawContainerSize(opusFile) *
        MadeUpOpusPrecisionFromCompressionRatio /
        decodedByteCount
      );
    }

    // Opus decodes to float, so this is the native format. However, libopus can decode
    // to 16-bit integers and there even is the possibility to compile libopus without
    // floating point code for embedded systems, so... maybe dig deeper?
    target.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_32;

  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t OpusReader::CountTotalFrames() const {
    return Platform::OpusApi::CountSamples(opusFile);
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> OpusReader::GetChannelOrder() const {
    const ::OpusHead &header = Platform::OpusApi::GetHeader(this->opusFile);

    return Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(
      header.mapping_family, header.channel_count
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t OpusReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::Seek(std::uint64_t frameIndex) {

    // CHECK: Could PCM offset mean interleaved sample index or is it a frame index?
    Platform::OpusApi::PcmSeek(this->opusFile, frameIndex);
    this->frameCursor = frameIndex;

  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::DecodeInterleaved(std::uint8_t *buffer, std::size_t frameCount) {
    decodeOpusAndConvert(
      this->opusFile, this->channelCount, this->frameCursor,
      buffer, frameCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::DecodeInterleaved(std::int16_t *buffer, std::size_t frameCount) {
    decodeOpusAndConvert(
      this->opusFile, this->channelCount, this->frameCursor,
      buffer, frameCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::DecodeInterleaved(std::int32_t *buffer, std::size_t frameCount) {
    decodeOpusAndConvert(
      this->opusFile, this->channelCount, this->frameCursor,
      buffer, frameCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::DecodeInterleaved(float *buffer, std::size_t frameCount) {
    while(frameCount >= 1) {

      // Decode the audio samples directly into the caller-provided buffer,
      // since we have a perfect match of the audio sample format.
      std::size_t decodedFrameCount = Platform::OpusApi::ReadFloat(
        this->opusFile, buffer, static_cast<int>(frameCount * this->channelCount)
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Opus file. File truncated?"
        );
      }

      this->frameCursor += decodedFrameCount;

      buffer += decodedFrameCount * this->channelCount;
      if(decodedFrameCount > frameCount) {
        assert((frameCount >= decodedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }

      frameCount -= decodedFrameCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusReader::DecodeInterleaved(double *buffer, std::size_t frameCount) {
    decodeOpusAndConvert(
      this->opusFile, this->channelCount, this->frameCursor,
      buffer, frameCount
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
