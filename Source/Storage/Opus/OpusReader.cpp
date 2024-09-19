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

#include "Nuclex/Audio/Storage/VirtualFile.h"

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

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
