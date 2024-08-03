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

#ifndef NUCLEX_AUDIO_CHANNELINFO_H
#define NUCLEX_AUDIO_CHANNELINFO_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <cstddef> // for std::size_t
#include <optional> // for std::optional
#include <string> // for std::string

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Informations about an audio channel</summary>
  struct NUCLEX_AUDIO_TYPE ChannelInfo {

    // Is there anything important here?
    // Does this class even have a justification to exist?
    //
    // -> If an audio container has multiple channels with the same placement (WTF?) or
    // channels with placements that our enum can't represent, would there be anything
    // worthwhile we could say about those? Otherwise, a ChannelCount would suffice...

    /// <summary>From where the channel will be played back</summary>
    public: ChannelPlacement Placement;


  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_CHANNELINFO_H
