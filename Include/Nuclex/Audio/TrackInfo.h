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

#ifndef NUCLEX_AUDIO_TRACKINFO_H
#define NUCLEX_AUDIO_TRACKINFO_H

#include "Nuclex/Audio/Config.h"

#include <cstddef> // for std::size_t
#include <optional> // for std::optional
#include <string> // for std::string

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Informations about an audio track (containing one or more channels)</summary>
  /// <remarks>
  ///   This structure is returned if you ask a codec to provide informations about
  ///   an audio file before actually loading it.
  /// </remarks>
  struct NUCLEX_AUDIO_TYPE TrackInfo {

    /// <summary>The name of the audio track, if provided by the container</summary>
    /// <remarks>
    ///   For single track containers such as .wav, .flac or .opus, there will only be
    ///   a track name if the file uses music tagging. For multi-track containers such
    ///   as .mka, there is a wealth of information about each audio track, usually
    ///   also including a human-readable name. Use to aid in selection by the user,
    ///   but anticipate that this string may be missing.
    /// </remarks>
    public: std::optional<std::string> Name;

    /// <summary>The language of the audio track in rfc-5646 format</summary>
    /// <remarks>
    ///   The rfc-5646 format defines language tags as you may have seen in mkv
    ///   files and other uses on the internet. They look like this: en-us,
    ///   en-uk or de-de (or just de).
    /// </remarks>
    public: std::optional<std::string> LanguageCode;

    // Expose channel count or channel infos?
    // Put sample rate in channel or here (forcing all to have the same)?
    // Put length in channel or here (same issue)?
    //   -> Audio editors like Audacity allow different lengths per channel
    //   -> Audio formats usually don't

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_TRACKINFO_H
