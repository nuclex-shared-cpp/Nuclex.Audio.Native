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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/TrackInfo.h"
#include "../../Platform/FlacApi.h"

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility class with intermediate methods used to decode FLAC files</summary>
  class FlacReader {

    /// <summary>Determines the native sample format from Flac's parameters</summary>
    /// <param name="bitsPerSample">The number of valid bits in each sample</param>
    /// <returns>The equivalent sample format enumeration value</returns>
    public: static AudioSampleFormat SampleFormatFromBitsPerSample(int bitsPerSample);

    /// <summary>
    ///   Determines the channel placement from the channel count and assignment
    /// </summary>
    /// <param name="channelCount">Number of audio channels in the FLAC file</param>
    /// <param name="channelAssignments">Channel assignment set that is used</param>
    /// <returns>The equivalent ChannelPlacement flag combination</returns>
    /// <remarks>
    ///   FLAC defines standard channel layouts for each possible number of channels up
    ///   to 8 (IETF draft FLAC specification 9.1.3). These can be overriden via a Vorbis
    ///   comment tag, but so long as the channel layout being encoded matches the standard
    ///   layout, said tag will be omitted. This method will return the channel placements
    ///   given the number of channels and their channel assignment bits.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromChannelCountAndAssignment(
      std::size_t channelCount, ::FLAC__ChannelAssignment channelAssignments
    );

    /// <summary>Parses the channel placement from the Vorbis comment tag</summary>
    /// <param name="channelMaskValue">Channel mask assigned via a Vorbis comment tag</param>
    /// <returns>The channel placement parsed from the Vorbis comment tag</returns>
    /// <remarks>
    ///   If the channel layout in a FLAC file deviates from the standard layout defined
    ///   per channel count, a Vorbis comment assigning the channel layout can be added as
    ///   Vorbis comment with the key <code>WAVEFORMATEXTENSIBLE_CHANNEL_MASK</code> and
    ///   matching Microsoft's channel mask bits from the Waveform file format
    ///   (IETF draft FLAC specification 8.6.2). These will match out ChannelPlacement
    ///   enum exactly, so, if present, the channel placements from this tag should be used.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromWaveFormatExtensibleTag(
      const std::string_view &channelMaskValue
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H
