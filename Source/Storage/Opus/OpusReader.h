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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/ChannelPlacement.h"

#include <string> // for std::string
#include <cstddef> // for std::size_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading OPUS files using libopus</summary>
  class OpusReader {

    /// <summary>
    ///   Determines the channel placement from the mapping family and channel count
    /// </summary>
    /// <param name="mappingFamily">Vorbis mapping family the channels conform to</param>
    /// <param name="channelCount">Number of audio channels in the FLAC file</param>
    /// <returns>The equivalent ChannelPlacement flag combination</returns>
    /// <remarks>
    ///   Opus shares the channel layouts and channel order with its predecessor codec,
    ///   Vorbis. There is a choice of mapping families (with only one actually being
    ///   really used so far) and the channel layout is fixed for each channel count.
    ///   The channel placements are all in section 4.3.9 of the Vorbis 1 Specification.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromMappingFamilyAndChannelCount(
      int mappingFamily, std::size_t channelCount
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H
