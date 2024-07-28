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

#include "Nuclex/Audio/ChannelPlacement.h"

#include <Nuclex/Support/BitTricks.h> // for BitTricks

namespace {

  // ------------------------------------------------------------------------------------------- //

  const std::string channelNames[] = {
    u8"unknown",
    u8"front left",
    u8"front right",
    u8"front center",
    u8"low frequency effects",
    u8"back left",
    u8"back right",
    u8"front center left",
    u8"front center right",
    u8"back center",
    u8"side left",
    u8"side right",
    u8"top center",
    u8"top front left",
    u8"top front center",
    u8"top front right",
    u8"top back left",
    u8"top back center",
    u8"top back right"
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  std::string StringFromChannelPlacement(ChannelPlacement placement) {
    std::string result;
    {
      std::size_t channelCount = Nuclex::Support::BitTricks::CountBits(
        static_cast<std::size_t>(placement) & 0x3FFFF
      );
      result.reserve(channelCount * 12); // channel names average 12 characters
    }

    for(std::size_t bitIndex = 0; bitIndex < 17; ++bitIndex) {
      if((static_cast<std::size_t>(placement) & (1 << bitIndex)) != 0) {
        if(result.empty()) {
          result.append(channelNames[bitIndex]);
        } else {
          result.append(u8", ", 2);
          result.append(channelNames[bitIndex]);
        }
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement ChannelPlacementFromString(
    const std::string &channelPlacementAsText
  ) {

  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio
