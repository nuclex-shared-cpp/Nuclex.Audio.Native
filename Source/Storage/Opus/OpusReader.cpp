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
#include "Nuclex/Audio/Processing/Quantization.h"

#include <numeric> // for std::gcd()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
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

  template<> void OpusReader::DecodeInterleaved<std::uint8_t>(
    std::uint8_t *target, std::size_t frameCount
  ) {
    decodeInterleavedAndConvert(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<std::int16_t>(
    std::int16_t *target, std::size_t frameCount
  ) {
    decodeInterleavedAndConvert(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<std::int32_t>(
    std::int32_t *target, std::size_t frameCount
  ) {
    decodeInterleavedAndConvert(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<float>(
    float *target, std::size_t frameCount
  ) {
    while(0 < frameCount) {

      // Decode the audio samples directly into the caller-provided buffer,
      // since we have a perfect match of the audio sample format.
      std::size_t decodedFrameCount = Platform::OpusApi::ReadFloat(
        this->opusFile, target, static_cast<int>(frameCount * this->channelCount)
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Opus file. File truncated?"
        );
      }

      this->frameCursor += decodedFrameCount;

      target += decodedFrameCount * this->channelCount;
      if(decodedFrameCount > frameCount) {
        assert((frameCount >= decodedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }

      frameCount -= decodedFrameCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<double>(
    double *target, std::size_t frameCount
  ) {
    decodeInterleavedAndConvert(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<std::uint8_t>(
    std::uint8_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<std::int16_t>(
    std::int16_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<std::int32_t>(
    std::int32_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<float>(
    float *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<double>(
    double *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void OpusReader::decodeInterleavedAndConvert(TSample *target, std::size_t frameCount) {
    std::vector<float> decodeBuffer(1020 * this->channelCount);

    // Keep going until we delivered all requested frames
    while(0 < frameCount) {

      // Decode into a float buffer which we will later convert into the target type.
      // We always use the float decoding method, even for int16, because the int16
      // decoding method in libopusfile does dithering. That's nice for playback,
      // but for this library's output, we don't want one audio format to dither and
      // another to not dither.
      std::size_t decodedFrameCount = Nuclex::Audio::Platform::OpusApi::ReadFloat(
        opusFile,
        decodeBuffer.data(),
        static_cast<int>(decodeBuffer.size() * this->channelCount)
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
      this->frameCursor += decodedFrameCount;

      // Either convert the decoded floats to doubles or quantize them into integers,
      // depending on the target type. We could just leave it up to SampleConverter::Convert(),
      // but at this point, we know the correct operation at compile time.
      std::size_t sampleCount = decodedFrameCount * this->channelCount;
      if constexpr(std::is_same<TSample, double>::value) {
        for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
          target[sampleIndex] = static_cast<double>(decodeBuffer[sampleIndex]);
        }
        target += sampleCount;
      } else { // if target type is double / integer
        typedef typename std::conditional<
          sizeof(TSample) < 17, float, double
        >::type LimitType;

        const float *source = decodeBuffer.data();
        LimitType limit = static_cast<LimitType>(
          (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
        );
        while(3 < sampleCount) {
          std::int32_t scaled[4];
          Nuclex::Audio::Processing::Quantization::MultiplyToNearestInt32x4(
            source, limit, scaled
          );
          target[0] = static_cast<TSample>(scaled[0]);
          target[1] = static_cast<TSample>(scaled[1]);
          target[2] = static_cast<TSample>(scaled[2]);
          target[3] = static_cast<TSample>(scaled[3]);
          source += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = static_cast<TSample>(
            Nuclex::Audio::Processing::Quantization::NearestInt32(
              static_cast<LimitType>(source[0]) * limit
            )
          );
          ++source;
          ++target;
          --sampleCount;
        }
      } // if target type is double / integer

      // Buffer was filled by the sample conversion method above, update the buffer pointer
      // to point to where the next batch of samples needs to be written
      if(decodedFrameCount > frameCount) {
        assert((frameCount >= decodedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }

      frameCount -= decodedFrameCount;

    } // while frames left to decode
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void OpusReader::decodeInterleavedConvertAndSeparate(
    TSample *targets[], std::size_t frameCount
  ) {
    constexpr bool targetTypeIsFloat = (
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value
    );

    // The channel pointers are in a caller-provided array, if we changed them,
    // we'd trash the caller's own pointers, so we have to take a copy.
    std::vector<TSample *> mutableTargets(this->channelCount);
    for(std::size_t index = 0; index < this->channelCount; ++index) {
      mutableTargets[index] = targets[index];
    }

    // Allocate an intermedia buffer. In this variant, we use std::byte because
    // we're going to be decoding into it as float, then quantizing to std::int32_t
    // in-place and we don't want to break any C++ aliasing rules.
    std::vector<std::byte> decodeBuffer(1020 * this->channelCount * sizeof(float));

    while(0 < frameCount) {

      // Decode into a float buffer which we will later convert into the target type.
      // We always use the float decoding method, even for int16, because the int16
      // decoding method in libopusfile does dithering. That's nice for playback,
      // but for this library's output, we don't want one audio format to dither and
      // another to not dither.
      std::size_t decodedFrameCount = Nuclex::Audio::Platform::OpusApi::ReadFloat(
        opusFile,
        reinterpret_cast<float *>(decodeBuffer.data()),
        static_cast<int>(decodeBuffer.size() * this->channelCount)
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

      // If the target type is not a floating point type, we need to quantize the values
      // to the range of the integer we're decoding into first
      if constexpr(!targetTypeIsFloat) {
        typedef typename std::conditional<
          sizeof(TSample) < 3, float, double
        >::type LimitType;

        LimitType limit = static_cast<LimitType>(
          (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
        );
        const float *decoded = reinterpret_cast<float *>(decodeBuffer.data());
        std::int32_t *target = reinterpret_cast<std::int32_t *>(decodeBuffer.data());

        std::size_t sampleCount = decodedFrameCount * this->channelCount;
        while(3 < sampleCount) {
          Nuclex::Audio::Processing::Quantization::MultiplyToNearestInt32x4(
            decoded, limit, target
          );
          decoded += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = static_cast<TSample>(
            Nuclex::Audio::Processing::Quantization::NearestInt32(
              static_cast<LimitType>(decoded[0]) * limit
            )
          );
          ++decoded;
          ++target;
          --sampleCount;
        }
      } // if samples need to be quantized

      // Sort the interleaved samples into each channel buffer. We know that a multiple
      // of the channel count was decoded (since op_read_float() returns the number of
      // frames), so we can simply run a nested loop to sort this out.
      {
        typedef typename std::conditional<
          targetTypeIsFloat, float, std::int32_t
        >::type DecodedType;
        DecodedType *decoded = reinterpret_cast<DecodedType *>(decodeBuffer.data());

        std::size_t sampleIndex = 0;
        for(std::size_t frameIndex = 0; frameIndex < decodedFrameCount; ++frameIndex) {
          for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              mutableTargets[channelIndex][frameIndex] = static_cast<TSample>(
                decoded[sampleIndex] + 128
              );
            } else {
              mutableTargets[channelIndex][frameIndex] = static_cast<TSample>(
                decoded[sampleIndex]
              );
            }
            ++sampleIndex;
          }
        }
      }

      // Advance the target buffer pointers
      for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
        mutableTargets[channelIndex] += decodedFrameCount;
      }

      frameCount -= decodedFrameCount;

    } // while frames left to decode
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
