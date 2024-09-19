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

}}}} // namespace Nuclex::Audio::Storage::Shared
