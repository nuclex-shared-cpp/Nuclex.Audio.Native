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

#include "./ChannelOrderTransformer.h"

#include <stdexcept> // for std::invalid_argument

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  bool ChannelOrderTransformer::IsWaveformatExtensibleLayout(
    const std::vector<ChannelPlacement> &channelOrder
  ) {
    std::size_t previousChannelBit = 0;

    // This is simple: since the WaveformatExtensible order is simply the order
    // of the channel mask bits, we only need to check if the values in the vector
    // are monotonic and unique.
    std::size_t placementCount = channelOrder.size();
    for(std::size_t index = 0; index < placementCount; ++index) {
      std::size_t currentChannelBit = static_cast<std::size_t>(channelOrder[index]);
      if(previousChannelBit >= currentChannelBit) {
        return false; // Channel is out of order to duplicate
      }
      previousChannelBit = currentChannelBit;
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_AUDIO_HAVE_VORBIS) || defined(NUCLEX_AUDIO_HAVE_OPUS)
  bool ChannelOrderTransformer::IsVorbisLayout(
    const std::vector<ChannelPlacement> &channelOrder
  ) {
    switch(channelOrder.size()) {
      case 1: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontCenter)
        );
      }
      case 2: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontRight)
        );
      }
      case 3: {
        return (
          (channelOrder[0] == ChannelPlacement::SideLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontCenter) &&
          (channelOrder[2] == ChannelPlacement::SideRight)
        );
      }
      case 4: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontRight) &&
          (channelOrder[2] == ChannelPlacement::BackLeft) &&
          (channelOrder[3] == ChannelPlacement::BackRight)
        );
      }
      case 5: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontCenter) &&
          (channelOrder[2] == ChannelPlacement::FrontRight) &&
          (channelOrder[3] == ChannelPlacement::BackLeft) &&
          (channelOrder[4] == ChannelPlacement::BackRight)
        );
      }
      case 6: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontCenter) &&
          (channelOrder[2] == ChannelPlacement::FrontRight) &&
          (channelOrder[3] == ChannelPlacement::BackLeft) &&
          (channelOrder[4] == ChannelPlacement::BackRight) &&
          (channelOrder[5] == ChannelPlacement::LowFrequencyEffects)
        );
      }
      case 7: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontCenter) &&
          (channelOrder[2] == ChannelPlacement::FrontRight) &&
          (channelOrder[3] == ChannelPlacement::SideLeft) &&
          (channelOrder[4] == ChannelPlacement::SideRight) &&
          (channelOrder[5] == ChannelPlacement::BackCenter) &&
          (channelOrder[6] == ChannelPlacement::LowFrequencyEffects)
        );
      }
      case 8: {
        return (
          (channelOrder[0] == ChannelPlacement::FrontLeft) &&
          (channelOrder[1] == ChannelPlacement::FrontCenter) &&
          (channelOrder[2] == ChannelPlacement::FrontRight) &&
          (channelOrder[3] == ChannelPlacement::SideLeft) &&
          (channelOrder[4] == ChannelPlacement::SideRight) &&
          (channelOrder[5] == ChannelPlacement::BackLeft) &&
          (channelOrder[6] == ChannelPlacement::BackRight) &&
          (channelOrder[7] == ChannelPlacement::LowFrequencyEffects)
        );
      }
      default: {
        return true; // Greater than eight channels: application defines usage
      }
    }
  }
#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS) || defined(NUCLEX_AUDIO_HAVE_OPUS)
  // ------------------------------------------------------------------------------------------- //

  std::vector<std::size_t> ChannelOrderTransformer::CreateRemappingTable(
    const std::vector<ChannelPlacement> &inputChannelOrder,
    const std::vector<ChannelPlacement> &targetChannelOrder
  ) {
    std::size_t inputChannelCount = inputChannelOrder.size();
    std::size_t targetChannelCount = targetChannelOrder.size();
    if(inputChannelCount != inputChannelCount) {
      throw std::invalid_argument(
        u8"Channel order mapping tables can only be created between same-sized channel lists"
      );
    }

    // Verify that the input channel order does not contain channels with unknown
    // placement. These cannot be mapped and will cause an exception to be thrown.
    for(std::size_t inputIndex = 0; inputIndex < inputChannelCount; ++inputIndex) {
      if(inputChannelOrder[inputIndex] == ChannelPlacement::Unknown) {
        throw std::invalid_argument(
          u8"Input channel order contains unknown channels"
        );
      }
    }

    // Prepare the remapping table and initialize it to invalid values. This way,
    // we can detect if a channel appears twice and that, after associating all
    // the channels, whether slots have been filled.
    std::vector<std::size_t> remappingIndices(targetChannelCount);
    for(std::size_t targetIndex = 0; targetIndex < targetChannelCount; ++targetIndex) {
      remappingIndices[targetIndex] = std::size_t(-1);
    }

    // Build the remapping table
    for(std::size_t targetIndex = 0; targetIndex < targetChannelCount; ++targetIndex) {
      ChannelPlacement targetChannel = targetChannelOrder[targetIndex];
      if(targetChannel == ChannelPlacement::Unknown) {
        throw std::invalid_argument(
          u8"Target channel order contains unknown channels"
        );
      }
      for(std::size_t inputIndex = 0; inputIndex < inputChannelCount; ++inputIndex) {
        if(inputChannelOrder[inputIndex] == targetChannel) {
          if(remappingIndices[inputIndex] != std::size_t(-1)) {
            throw std::invalid_argument(
              u8"Input channel order contains duplicate channels"
            );
          } // if remapping index already occupied

          remappingIndices[inputIndex] = inputIndex;
        } // if input channel matches target channel
      } // for each input channel
    } // for each target channel

    // Finally, the input channels may have been different (as in, contain wholly
    // different placements, not just a different ordering). In this case,
    // it is also impossible to build a remapping table and we error out.
    for(std::size_t targetIndex = 0; targetIndex < targetChannelCount; ++targetIndex) {
      if(remappingIndices[targetIndex] == std::size_t(-1)) {
        throw std::invalid_argument(
          u8"Target channel order contains channels not present in the input channel order"
        );
      }
    }

    return remappingIndices;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared
