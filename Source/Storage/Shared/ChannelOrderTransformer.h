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

#ifndef NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERTRANSFORMER_H
#define NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERTRANSFORMER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <cstddef> // for std::size_t
#include <vector> // for std::vector

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates remapping tables and reorders channels in interleaved audio</summary>
  class ChannelOrderTransformer {

    /// <summary>
    ///   Checks if the specified channel order matches the WaveformatExtensible order
    /// </summary>
    /// <param name="channelOrder">
    ///   Channel order that will be checked against the WaveformatExtensible order
    /// </param>
    /// <returns>True if the channel order matches the WaveformatExtensible order</returns>
    public: static bool IsWaveformatExtensibleLayout(
      const std::vector<ChannelPlacement> &channelOrder
    );

#if defined (NUCLEX_AUDIO_HAVE_VORBIS) || defined (NUCLEX_AUDIO_HAVE_OPUS)

    /// <summary>
    ///   Checks if the specified channel order matches the Vorbis order
    /// </summary>
    /// <param name="channelOrder">
    ///   Channel order that will be checked against the Vorbis order
    /// </param>
    /// <returns>True if the channel order matches the Vorbis order</returns>
    public: static bool IsVorbisLayout(
      const std::vector<ChannelPlacement> &channelOrder
    );

#endif // defined (NUCLEX_AUDIO_HAVE_VORBIS) || defined (NUCLEX_AUDIO_HAVE_OPUS)

    /// <summary>
    ///   Creates a table that stores the index of the input channel to use for
    ///   each output channel
    /// </summary>
    /// <param name="inputChannelOrder">Order in which the channels are stored</param>
    /// <param name="targetChannelOrder">Order to which the channels should be mapped</param>
    /// <returns>
    ///   A list of the corresponding input channel index for each output channel
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     The returned table holds the index of the input channel that corresponds
    ///     to each output channel. Mappings between mismatching numbers of channels
    ///     will not be generated.
    ///   </para>
    ///   <para>
    ///     <code>
    ///       std::vector&lt;std::size_t&gt; inputChannelLookup = (
    ///         ChannelOrderTransformer::CreateRemappingTable(inputs, outputs)
    ///       );
    ///       for(std::size_t channel = 0; channel < channelCount; ++index) {
    ///         audioOut[channel] = audioIn[inputChannelLookup[channel]];
    ///       }
    ///     </code>
    ///   </para>
    /// </remarks>
    public: static std::vector<std::size_t> CreateRemappingTable(
      const std::vector<ChannelPlacement> &inputChannelOrder,
      const std::vector<ChannelPlacement> &targetChannelOrder
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared

#endif // NUCLEX_AUDIO_STORAGE_SHARED_CHANNELORDERTRANSFORMER_H
