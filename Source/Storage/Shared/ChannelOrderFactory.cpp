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

#include "./ChannelOrderFactory.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> ChannelOrderFactory::FromWaveformatExtensibleLayout(
    std::size_t channelCount, ChannelPlacement channelPlacements
  ) {
    std::vector<ChannelPlacement> channelOrder;
    channelOrder.reserve(channelCount);

    // Add those channels that are part of the channel mask. These always come first,
    // with any unidentified channels following after. If the channel mask is empty,
    // this will simply do nothing (valid; see comment below).
    {
      for(std::size_t bitIndex = 0; bitIndex < 17; ++bitIndex) {
        if((static_cast<std::size_t>(channelPlacements) & (1ULL << bitIndex)) != 0) {
          channelOrder.push_back(static_cast<ChannelPlacement>(1ULL << bitIndex));
          --channelCount;
        }
      }
    }

    // In WAVEFORMATEXTENSIBLE (and therefore in WavPack?) it is valid to set the channel mask
    // flags to zero and include channels. These are then arbitrary, non-placeable channels
    // not associated with specific speakers. In such a case, or if the channel count exceeds
    // the number of channel mask bits set, we add the remaining channels as unknown channels.
    while(channelCount >= 1) {
      channelOrder.push_back(ChannelPlacement::Unknown);
      --channelCount;
    }

    return channelOrder;
  }

  // ------------------------------------------------------------------------------------------- //
#if defined (NUCLEX_AUDIO_HAVE_VORBIS) || defined (NUCLEX_AUDIO_HAVE_OPUS)
  std::vector<ChannelPlacement> ChannelOrderFactory::FromVorbisFamilyAndCount(
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
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined (NUCLEX_AUDIO_HAVE_VORBIS) || defined (NUCLEX_AUDIO_HAVE_OPUS)
  ChannelPlacement ChannelOrderFactory::ChannelPlacementFromVorbisFamilyAndCount(
    int mappingFamily, std::size_t channelCount
  ) {

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
#endif
  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared
