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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACDETECTION_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACDETECTION_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid FLAC file possible</summary>
  /// <remarks>
  ///   Determined by generating a mono file with 1 audio sample in Audacity, then
  ///   trimming the encoder version string.
  /// </remarks>
  constexpr const std::size_t SmallestPossibleFlacSize = 50; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for detecting FLAC files using libFLAC</summary>
  class Detection {

    /// <summary>Checks if the specified file extension indicates a .flac file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates a .flac file</returns>
    public: static bool DoesFileExtensionSayFlac(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid FLAC header</summary>
    /// <param name="source">File that will be checked for a valid FLAC header</param>
    /// <returns>True if a valid FLAC header was found, false otherwise</returns>
    public: static bool CheckIfFlacHeaderPresent(const VirtualFile &source);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACDETECTION_H
