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

#ifndef NUCLEX_AUDIO_CONTAINERINFO_H
#define NUCLEX_AUDIO_CONTAINERINFO_H

#include "Nuclex/Audio/Config.h"

#include <vector> // for std::vector
#include <cstddef> // for std::size_t
#include <optional> // for std::optional
#include <string> // for std::string

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  class TrackInfo;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Informations about a media container holding one or more audio tracks</summary>
  /// <remarks>
  ///   <para>
  ///     Media containers can hold encoded audio streams, similar to how a .zip archive
  ///     with level 0 (= don't compress) compression would hold data files, except that
  ///     the data is stored interleaved. Some audio formats are their own containers
  ///     and can/must contain exactly one audio stream. Classic .wav and .mp3 files are
  ///     such cases, as are .flac and .wv (WavPack).
  ///   </para>
  ///   <para>
  ///     If information for these is queried as a container, a container holding exactly
  ///     one audio track will be returned.
  ///   </para>
  ///   <para>
  ///     Then there are actual media containers that can hold multiple audio tracks.
  ///     These include .mka (Matroska audio, can store streams of almost any audio codec),
  ///     .ogg (default container for Vorbis, but can also store OPUS and FLAC as well)
  ///     and .m4a (same as .mp4 and typically holds one or more AC3 or AAC tracks).
  ///   </para>
  ///   <para>
  ///     If these are queried as a container, all the container's audio tracks are listed,
  ///     of course. If you query these as a single track, the informations will be of
  ///     the default track. That is either the first audio track with the default flag
  ///     set (in media container formats where such a flag exists) or simple the first
  ///     audio track listed in the container.
  ///   </para>
  /// </remarks>
  struct NUCLEX_AUDIO_TYPE ContainerInfo {

    /// <summary>Index of the default track in the container</summary>
    public: std::size_t DefaultTrackIndex;

    /// <summary>Audio tracks in this container</summary>
    public: std::vector<TrackInfo> Tracks;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_CONTAINERINFO_H
