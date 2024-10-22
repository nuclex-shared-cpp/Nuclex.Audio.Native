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

#ifndef NUCLEX_AUDIO_KNOWNCHANNELLAYOUTS_H
#define NUCLEX_AUDIO_KNOWNCHANNELLAYOUTS_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides channel placements for most standard channel layouts</summary>
  class KnownChannelLayouts {

    /// <summary>
    ///   Two channels either intended for speaker placed diagonally in front of the listener
    ///   or for headphones
    /// </summary>
    public: static const ChannelPlacement Stereo;

    /// <summary>Placements of channels in a typical 5.1 surround layout</summary>
    /// <remarks>
    ///   This is the most common channel layout. Some codecs used in movie audio tracks
    ///   (DTS, PCM, FLAC and WavPack) are able to distinguish between this layout,
    ///   using rear speakers, and a similar layout that uses side speakers instead.
    ///   The Opus and Vorbis codecs, on the other hand, only know this layout for 5.1.
    /// </remarks>
    public: static const ChannelPlacement FiveDotOneSurround;

    /// <summary>Placements of channels in a 5.1 surround layout with side speakers</summary>
    public: static const ChannelPlacement FiveDotOneSideSurround;

    /// <summary>Placements of channels in a standard 7.1 surround layout</summary>
    public: static const ChannelPlacement SevenDotOneSurround;

  };

  // ------------------------------------------------------------------------------------------- //

  inline const ChannelPlacement KnownChannelLayouts::Stereo = (
    ChannelPlacement::FrontLeft |
    ChannelPlacement::FrontRight
  );

  // ------------------------------------------------------------------------------------------- //

  inline const ChannelPlacement KnownChannelLayouts::FiveDotOneSurround = (
    ChannelPlacement::FrontLeft |
    ChannelPlacement::FrontRight |
    ChannelPlacement::FrontCenter |
    ChannelPlacement::LowFrequencyEffects |
    ChannelPlacement::BackLeft |
    ChannelPlacement::BackRight
  );

  // ------------------------------------------------------------------------------------------- //

  inline const ChannelPlacement KnownChannelLayouts::FiveDotOneSideSurround = (
    ChannelPlacement::FrontLeft |
    ChannelPlacement::FrontRight |
    ChannelPlacement::FrontCenter |
    ChannelPlacement::LowFrequencyEffects |
    ChannelPlacement::SideLeft |
    ChannelPlacement::SideRight
  );

  // ------------------------------------------------------------------------------------------- //

  inline const ChannelPlacement KnownChannelLayouts::SevenDotOneSurround = (
    ChannelPlacement::FrontLeft |
    ChannelPlacement::FrontRight |
    ChannelPlacement::FrontCenter |
    ChannelPlacement::LowFrequencyEffects |
    ChannelPlacement::BackLeft |
    ChannelPlacement::BackRight |
    ChannelPlacement::SideLeft |
    ChannelPlacement::SideRight
  );

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_KNOWNCHANNELLAYOUTS_H
