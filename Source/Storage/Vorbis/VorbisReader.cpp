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

#include "./VorbisReader.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "./VorbisVirtualFileAdapter.h" // for FileAdapterFactory
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/VorbisApi.h" // for VorbisApi

#include "Nuclex/Audio/Errors/CorruptedFileError.h"
#include "Nuclex/Audio/Processing/Quantization.h"

#include "Nuclex/Audio/TrackInfo.h"

#include <stdexcept> // for std::runtime_error
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  VorbisReader::VorbisReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    fileCallbacks(),
    state(),
    vorbisFile(),
    channelCount(0),
    frameCursor(0) {

    // Set up the libvorbisfile callbacks with adapter methods that will perform all reads
    // on the user-provided virtual file.
    this->state = (
      FileAdapterFactory::CreateAdapterForReading(file, this->fileCallbacks)
    );

    // Open the Vorbis file, obtaining a OggVorbisFile instance. Everything inside
    // this scope is just error plumbing code, ensuring that the right exception
    // surfaces if either libvorbisfile reports an error or the virtual file throws.
    this->vorbisFile = Platform::VorbisApi::OpenFromCallbacks(
      state->Error,
      state.get(),
      fileCallbacks
    );

    // The OpenFromCallbacks() method will already have checked for errors,
    // but if some file access error happened that libvorbisfile deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    FileAdapterState::RethrowPotentialException(*state);

    // Vorbis audio streams can be chained together (sequentially and not in the sense of
    // interleaving it as another stream in the OGG container). This would mean that
    // the audio stream properties (i.e. channel count, sample rate) might change while
    // we are decoding...
    //
    std::size_t streamCount = Platform::VorbisApi::CountStreams(this->vorbisFile);
    if(streamCount != 1) {
      throw std::runtime_error(u8"Multi-stream Vorbis files are not supported");
    }

    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);
    this->channelCount = info.channels;

  }

  // ------------------------------------------------------------------------------------------- //

  VorbisReader::~VorbisReader() {}

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::ReadMetadata(TrackInfo &target) {
    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);

    // For Vorbis files, the mapping family is always 0 (says the Vorbis 1 specification)
    target.ChannelCount = info.channels;
    target.ChannelPlacements = (
      Shared::ChannelOrderFactory::ChannelPlacementFromVorbisFamilyAndCount(
        0, target.ChannelCount
      )
    );

    target.SampleRate = info.rate;

    ::ogg_int64_t totalSampleCount = Platform::VorbisApi::CountPcmSamples(this->vorbisFile);

    target.Duration = std::chrono::microseconds(
      totalSampleCount * 1'000'000 / target.SampleRate
    );

    target.BitsPerSample = 15; // come up with a silly, wrong approximation formula here

    target.SampleFormat = AudioSampleFormat::Float_32;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::CountTotalFrames() const {
    return Platform::VorbisApi::CountPcmSamples(this->vorbisFile);
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> VorbisReader::GetChannelOrder() const {
    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);

    return Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(
      0, info.channels
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::Seek(std::uint64_t frameIndex) {
    Platform::VorbisApi::Seek(
      this->state->Error,
      this->vorbisFile,
      frameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeInterleaved<std::uint8_t>(
    std::uint8_t *target, std::size_t frameCount
  ) {
    decodeSeparatedConvertAndInterleave(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeInterleaved<std::int16_t>(
    std::int16_t *target, std::size_t frameCount
  ) {
    decodeSeparatedConvertAndInterleave(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeInterleaved<std::int32_t>(
    std::int32_t *target, std::size_t frameCount
  ) {
    decodeSeparatedConvertAndInterleave(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeInterleaved<float>(
    float *target, std::size_t frameCount
  ) {
    decodeSeparatedConvertAndInterleave(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeInterleaved<double>(
    double *target, std::size_t frameCount
  ) {
    decodeSeparatedConvertAndInterleave(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeSeparated<std::uint8_t>(
    std::uint8_t *targets[], std::size_t frameCount
  ) {
    decodeSeparatedAndConvert(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeSeparated<std::int16_t>(
    std::int16_t *targets[], std::size_t frameCount
  ) {
    decodeSeparatedAndConvert(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeSeparated<std::int32_t>(
    std::int32_t *targets[], std::size_t frameCount
  ) {
    decodeSeparatedAndConvert(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeSeparated<float>(
    float *targets[], std::size_t frameCount
  ) {
    decodeSeparatedAndConvert(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void VorbisReader::DecodeSeparated<double>(
    double *targets[], std::size_t frameCount
  ) {
    decodeSeparatedAndConvert(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void VorbisReader::decodeSeparatedConvertAndInterleave(
    TSample *target, std::size_t frameCount
  ) {

    while(0 < frameCount) {

      // Unfortunately, libvorbisfile will not decode into a buffer we provide,
      // but hand out its own buffers. This means we are unable to decode directly
      // into caller-provided buffers and will, at the minimum, have to do a copy.
      float **samples = nullptr;
      int streamIndex = -1;
      std::size_t decodedFrameCount = Platform::VorbisApi::ReadFloat(
        this->state->Error,
        this->vorbisFile,
        samples,
        static_cast<int>(std::min<std::size_t>(frameCount, 8192)), // should be maximum frame size anyways
        streamIndex
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Vorbis file. File truncated?"
        );
      }

      // Since libvorbisfile deigned the code a success, we need to take record of
      // the new frame cursor position even if we fail the decode due to some other
      // issue in the code that follows.
      this->frameCursor += decodedFrameCount;

      if(streamIndex != 0) {
        throw std::runtime_error(
          u8"Vorbis decoding reached another stream. Multi-stream files are not supported."
        );
      }

      constexpr bool targetIsFloat = (
        std::is_same<TSample, float>::value ||
        std::is_same<TSample, double>::value
      );
      if constexpr(targetIsFloat) {
        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          TSample *channelTarget = target + channelIndex;
          float *source = samples[channelIndex];
          for(std::size_t sampleIndex = 0; sampleIndex < decodedFrameCount; ++sampleIndex) {
            *channelTarget = static_cast<TSample>(source[sampleIndex]);
            channelTarget += this->channelCount;
          }
        }
      } else { // float -> integer
        typedef typename std::conditional<
          sizeof(TSample) < 3, float, double
        >::type LimitType;
        LimitType limit = static_cast<LimitType>(
          (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
        );

        std::size_t channelCountTimes2 = this->channelCount + this->channelCount;
        std::size_t channelCountTimes3 = channelCountTimes2 + this->channelCount;
        std::size_t channelCountTimes4 = channelCountTimes3 + this->channelCount;

        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          std::size_t sampleCount = decodedFrameCount;
          TSample *channelTarget = target + channelIndex;
          float *source = samples[channelIndex];

          while(3 < sampleCount) {
            std::int32_t scaled[4];
            Processing::Quantization::MultiplyToNearestInt32x4(
              source, limit, scaled
            );

            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              channelTarget[0] = static_cast<TSample>(scaled[0] + 128);
              channelTarget[this->channelCount] = static_cast<TSample>(scaled[1] + 128);
              channelTarget[channelCountTimes2] = static_cast<TSample>(scaled[2] + 128);
              channelTarget[channelCountTimes3] = static_cast<TSample>(scaled[3] + 128);
            } else {
              channelTarget[0] = static_cast<TSample>(scaled[0]);
              channelTarget[this->channelCount] = static_cast<TSample>(scaled[1]);
              channelTarget[channelCountTimes2] = static_cast<TSample>(scaled[2]);
              channelTarget[channelCountTimes3] = static_cast<TSample>(scaled[3]);
            }

            source += 4;
            channelTarget += channelCountTimes4;
            sampleCount -=4;
          }
          while(0 < sampleCount) {
            std::int32_t scaled[4];
            Processing::Quantization::MultiplyToNearestInt32x4(
              source, limit, scaled
            );

            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              channelTarget[0] = static_cast<TSample>(
                Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(source[0]) * limit
                ) + 128
              );
            } else {
              channelTarget[0] = static_cast<TSample>(
                Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(source[0]) * limit
                )
              );
            }

            ++source;
            channelTarget += this->channelCount;
            --sampleCount;
          }
        }
      }

      target += decodedFrameCount * this->channelCount;

      frameCount -= decodedFrameCount;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void VorbisReader::decodeSeparatedAndConvert(
    TSample *targets[], std::size_t frameCount
  ) {

    // The channel pointers are in a caller-provided array, if we changed them,
    // we'd trash the caller's own pointers, so we have to take a copy.
    std::vector<TSample *> mutableTargets(this->channelCount);
    for(std::size_t index = 0; index < this->channelCount; ++index) {
      mutableTargets[index] = targets[index];
    }

    while(0 < frameCount) {

      // Unfortunately, libvorbisfile will not decode into a buffer we provide,
      // but hand out its own buffers. This means we are unable to decode directly
      // into caller-provided buffers and will, at the minimum, have to do a copy.
      float **samples = nullptr;
      int streamIndex = -1;
      std::size_t decodedFrameCount = Platform::VorbisApi::ReadFloat(
        this->state->Error,
        this->vorbisFile,
        samples,
        static_cast<int>(frameCount), // * this->channelCount,
        streamIndex
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Vorbis file. File truncated?"
        );
      }

      // Since libvorbisfile deigned the code a success, we need to take record of
      // the new frame cursor position even if we fail the decode due to some other
      // issue in the code that follows.
      this->frameCursor += decodedFrameCount;

      if(streamIndex != 0) {
        throw std::runtime_error(
          u8"Vorbis decoding reached another stream. Multi-stream files are not supported."
        );
      }

      // Floating point can be memory-copied into the output buffers unchanged
      if constexpr(std::is_same<TSample, float>::value) {
        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          std::copy_n(samples[channelIndex], decodedFrameCount, mutableTargets[channelIndex]);
          mutableTargets[channelIndex] += decodedFrameCount;
        }
      } else if constexpr(std::is_same<TSample, double>::value) { // float -> double
        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          double *target = mutableTargets[channelIndex];
          float *source = samples[channelIndex];
          for(std::size_t sampleIndex = 0; sampleIndex < decodedFrameCount; ++sampleIndex) {
            target[sampleIndex] = static_cast<double>(source[sampleIndex]);
          }

          mutableTargets[channelIndex] += decodedFrameCount;
        }
      } else { // float -> integer
        typedef typename std::conditional<
          sizeof(TSample) < 3, float, double
        >::type LimitType;
        LimitType limit = static_cast<LimitType>(
          (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
        );

        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          std::size_t sampleCount = decodedFrameCount;
          TSample *target = mutableTargets[channelIndex];
          float *source = samples[channelIndex];

          while(3 < sampleCount) {
            std::int32_t scaled[4];
            Processing::Quantization::MultiplyToNearestInt32x4(
              source, limit, scaled
            );

            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              target[0] = static_cast<TSample>(scaled[0] + 128);
              target[1] = static_cast<TSample>(scaled[1] + 128);
              target[2] = static_cast<TSample>(scaled[2] + 128);
              target[3] = static_cast<TSample>(scaled[3] + 128);
            } else {
              target[0] = static_cast<TSample>(scaled[0]);
              target[1] = static_cast<TSample>(scaled[1]);
              target[2] = static_cast<TSample>(scaled[2]);
              target[3] = static_cast<TSample>(scaled[3]);
            }

            source += 4;
            target += 4;
            sampleCount -=4;
          }
          while(0 < sampleCount) {
            std::int32_t scaled[4];
            Processing::Quantization::MultiplyToNearestInt32x4(
              source, limit, scaled
            );

            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              target[0] = static_cast<TSample>(
                Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(source[0]) * limit
                ) + 128
              );
            } else {
              target[0] = static_cast<TSample>(
                Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(source[0]) * limit
                )
              );
            }

            ++source;
            ++target;
            --sampleCount;
          }

          mutableTargets[channelIndex] += decodedFrameCount;
        }
      }

      frameCount -= decodedFrameCount;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
