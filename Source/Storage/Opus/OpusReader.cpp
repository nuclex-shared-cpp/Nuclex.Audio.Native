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
#include "../../Platform/OpusApi.h" // for OpusApi

#include "Nuclex/Audio/TrackInfo.h"

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <cmath>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  Nuclex::Audio::ChannelPlacement OpusReader::ChannelPlacementFromMappingFamilyAndChannelCount(
    int mappingFamily, std::size_t channelCount
  ) {
    using Nuclex::Audio::ChannelPlacement;

    // Opus uses the Vorbis channel layouts and orders. These can be found in section 4.3.9
    // of the Vorbis 1 Specification (if you cloned the repository this file is in, you'll
    // find a copy of said specification in its Documents directory).
    //
    if((mappingFamily == 0) || (mappingFamily == 1)) {
      switch(channelCount) {
        case 1: {
          return ChannelPlacement::FrontCenter;
        }
        case 2: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontRight
          );
        }
        case 3: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight
          );
        }
        case 4: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight |
            ChannelPlacement::BackCenter
          );
        }
        case 5: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight |
            ChannelPlacement::BackLeft |
            ChannelPlacement::BackRight
          );
        }
        case 6: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight |
            ChannelPlacement::BackLeft |
            ChannelPlacement::BackRight |
            ChannelPlacement::LowFrequencyEffects
          );
        }
        case 7: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight |
            ChannelPlacement::SideLeft |
            ChannelPlacement::SideRight |
            ChannelPlacement::BackCenter |
            ChannelPlacement::LowFrequencyEffects
          );
        }
        case 8: {
          return (
            ChannelPlacement::FrontLeft |
            ChannelPlacement::FrontCenter |
            ChannelPlacement::FrontRight |
            ChannelPlacement::SideLeft |
            ChannelPlacement::SideRight |
            ChannelPlacement::BackLeft |
            ChannelPlacement::BackRight |
            ChannelPlacement::LowFrequencyEffects
          );
        }
        default: {
          return ChannelPlacement::Unknown;
        }
      }
    } else { // family (0 | 1) / other family
      return ChannelPlacement::Unknown;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(
    int mappingFamily, std::size_t channelCount
  ) {
    std::vector<ChannelPlacement> channelOrder;
    channelOrder.reserve(channelCount);

    if((mappingFamily == 0) || (mappingFamily == 1)) {
      std::size_t originalChannelCount = channelCount;
      if(originalChannelCount == 1) {
        channelOrder.push_back(ChannelPlacement::FrontCenter);
        --channelCount;
      } else if(originalChannelCount < 9) {
        channelOrder.push_back(ChannelPlacement::FrontLeft);
        channelCount -= 2;

        if((originalChannelCount == 3) || (originalChannelCount >= 5)) {
          channelOrder.push_back(ChannelPlacement::FrontCenter);
          --channelCount;
        }

        channelOrder.push_back(ChannelPlacement::FrontRight);

        if(originalChannelCount >= 7) {
          channelOrder.push_back(ChannelPlacement::SideLeft);
          channelCount -= 2;
          channelOrder.push_back(ChannelPlacement::SideRight);
        }

        if(originalChannelCount == 7) {
          channelOrder.push_back(ChannelPlacement::BackCenter);
          --channelCount;
        }

        if((originalChannelCount >= 4) && (originalChannelCount != 7)) {
          channelOrder.push_back(ChannelPlacement::BackLeft);
          channelCount -= 2;
          channelOrder.push_back(ChannelPlacement::BackRight);
        }

        if(originalChannelCount >= 6) {
          channelOrder.push_back(ChannelPlacement::LowFrequencyEffects);
          --channelCount;
        }
      }
    }

    while(channelCount >= 1) {
      channelOrder.push_back(ChannelPlacement::Unknown);
      --channelCount;
    }

    return channelOrder;
  }

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

    target.ChannelPlacements = OpusReader::ChannelPlacementFromMappingFamilyAndChannelCount(
      header.mapping_family, target.ChannelCount
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

    return OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(
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

  void OpusReader::DecodeInterleaved(float *buffer, std::size_t frameCount) {
    while(frameCount >= 1) {
      std::size_t decodedFrameCount = (
        Platform::OpusApi::ReadFloat(this->opusFile, buffer, frameCount * this->channelCount)
      );
      if(decodedFrameCount == 0) {
        throw std::runtime_error(
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

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
