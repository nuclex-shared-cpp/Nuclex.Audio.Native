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

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Special value found in the bitsPerSample attribute when using floating point samples
  /// </summary>
  constexpr std::size_t FloatBitsPerSample(-32);

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat FlacTrackDecoder::SampleFormatFromBitsPerSample(int bitsPerSample) {

    // I'm not entirely sure if FLAC files even can have bits per sample that are
    // not a multiple of 8, but in the sense of defensive programming, we do range checks:
    if(bitsPerSample >= 25) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
    } else if(bitsPerSample >= 17) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
    } else if(bitsPerSample >= 9) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
    } else {
      return Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement FlacTrackDecoder::ChannelPlacementFromChannelCountAndAssignment(
    std::size_t channelCount, ::FLAC__ChannelAssignment channelAssignments
  ) {
    switch(channelAssignments) {
      case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT: {
        switch(channelCount) {
          case 1: {
            return (
              ChannelPlacement::FrontCenter
            );
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
              ChannelPlacement::FrontRight |
              ChannelPlacement::FrontCenter
            );
          }
          case 4: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontRight |
              ChannelPlacement::BackLeft |
              ChannelPlacement::BackRight
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
          default: { return ChannelPlacement::Unknown; }
        }
      }
      case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
      case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
      case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE: {
        return ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight;
      }
      default: {
        return ChannelPlacement::Unknown;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
