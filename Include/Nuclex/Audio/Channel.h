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

#ifndef NUCLEX_AUDIO_CHANNEL_H
#define NUCLEX_AUDIO_CHANNEL_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"
#include "Nuclex/Audio/AudioSampleFormat.h"

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Base class containing the shared attributes of single audio channels</summary>
  /// <remarks>
  ///   This base class aids in keeping attributes that do not depend on the sample type
  ///   from having multiple clones due to being in a templated class. Most modern containers
  ///   likely can avoid duplicating such code, but this has the added advantage of allowing
  ///   us to keep the implementation out of the header, too.
  /// </remarks>
  class NUCLEX_AUDIO_TYPE ChannelBase {

    /// <summary>Initializes a new audio channel</summary>
    public: NUCLEX_AUDIO_API ChannelBase() :
      placement() {}

    /// <summary>Frees all memory directly owned by the audio channel</summary>
    public: NUCLEX_AUDIO_API ~ChannelBase() = default;

    /// <summary>Retrieves the placement of the channel relative to the listener</summary>
    /// <returns>The channel's placement relative to the listener</returns>
    public: NUCLEX_AUDIO_API ChannelPlacement GetPlacement() const {
      return this->placement;
    }

    /// <summary>Updates the placement of the channel relative to the listener</summary>
    /// <param name="newPlacement">New placement of the audio channel</param>
    public: NUCLEX_AUDIO_API void SetPlacement(ChannelPlacement newPlacement);

    /// <summary>Retrieves the format in which audio samples are natively stored</summary>
    /// <returns>The native storage format of audio samples in this channel</returns>
    public: NUCLEX_AUDIO_API AudioSampleFormat GetNativeSampleFormat() const {
      return this->sampleFormat;
    }

    /*
    public: template<typename TOtherSample>
    std::shared_ptr<const Channel<TOtherSample>> As() const;

    public: template<typename TOtherSample>
    std::shared_ptr<Channel<TOtherSample>> Convert() const;

    */

    /// <summary>Where the audio channel should be played relative to the listener</summary>
    private: ChannelPlacement placement;
    /// <summary>The format in which audio samples are natively stored</summary>
    private: AudioSampleFormat sampleFormat;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_CHANNEL_H
