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

#ifndef NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERFACTORY_H
#define NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERFACTORY_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <cstddef> // for std::size_t
#include <vector> // for std::vector

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates channel ordering lists according to diffeent conventions</summary>
  class ChannelOrderFactory {

    /// <summary>
    ///   Generates an ordered channel list according to the conventions used by
    ///   Microsoft's Waveform / WaveFormatExtensible
    /// </summary>
    /// <param name="channelCount">Total number of channels in the file</param>
    /// <param name="channelPlacements">
    ///   Channel placements according to WaveFormatExtensible. The enumeration in this
    ///   library is compatible to that layout, so the parameter uses the enum right away.
    /// </param>
    /// <returns>A vector containing the audio channels in the interleaved order</returns>
    public: static std::vector<ChannelPlacement> FromWaveformatExtensibleLayout(
      std::size_t channelCount, ChannelPlacement channelPlacements
    );

#if defined (NUCLEX_AUDIO_HAVE_VORBIS) || defined (NUCLEX_AUDIO_HAVE_OPUS)

    /// <summary>
    ///   Generates an ordered channel list according to the conventions used by
    ///   the Vorbis specification (which also applies to Opus)
    /// </summary>
    /// <param name="mappingFamily">Vorbis mapping family the channels conform to</param>
    /// <param name="channelCount">Number of audio channels in the audiop file</param>
    /// <returns>A vector containing the audio channels in the interleaved order</returns>
    public: static std::vector<ChannelPlacement> FromVorbisFamilyAndCount(
      int mappingFamily, std::size_t channelCount
    );

    /// <summary>
    ///   Determines the channel placement from the mapping family and channel count
    /// </summary>
    /// <param name="mappingFamily">Vorbis mapping family the channels conform to</param>
    /// <param name="channelCount">Number of audio channels in the audio file</param>
    /// <returns>The equivalent ChannelPlacement flag combination</returns>
    /// <remarks>
    ///   Opus shares the channel layouts and channel order with its predecessor codec,
    ///   Vorbis. There is a choice of mapping families (with only one actually being
    ///   really used so far) and the channel layout is fixed for each channel count.
    ///   The channel placements are all in section 4.3.9 of the Vorbis 1 Specification.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromVorbisFamilyAndCount(
      int mappingFamily, std::size_t channelCount
    );

#endif // if Vorbis or Opus codecs enabled

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared

#endif // NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERFACTORY_H
