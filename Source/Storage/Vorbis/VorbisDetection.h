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

#ifndef NUCLEX_AUDIO_STORAGE_VORBIS_VORBISDETECTION_H
#define NUCLEX_AUDIO_STORAGE_VORBIS_VORBISDETECTION_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include <string> // for std::string
#include <cstddef> // for std::size_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid Vorbis file possible</summary>
  /// <remarks>
  ///   Unknown. The smallest file I can produce with oggenc is way over 4 kilobytes.
  ///   Some cursory research seems to indicate that Vorbis uses dynamic code tables,
  ///   which give it a base footprint even for very short audio streams.
  /// </remarks>
  constexpr const std::size_t SmallestPossibleVorbisSize = 4096; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for detecting the header of Ogg Vorbis files</summary>
  class Detection {

    /// <summary>Checks if the specified file extension indicates an .ogg file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates an .ogg file</returns>
    public: static bool DoesFileExtensionSayOgg(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid VORBIS header</summary>
    /// <param name="source">File that will be checked for a valid VORBIS header</param>
    /// <returns>True if a valid VORBIS header was found, false otherwise</returns>
    public: static bool CheckIfVorbisHeaderPresent(const VirtualFile &source);

    /// <summary>Checks if the specified file starts with a valid VORBIS header</summary>
    /// <param name="source">File that will be checked for a valid VORBIS header</param>
    /// <returns>True if a valid VORBIS header was found, false otherwise</returns>
    public: static bool CheckIfVorbisHeaderPresentLite(const VirtualFile &source);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)

#endif // NUCLEX_AUDIO_STORAGE_VORBIS_VORBISDETECTION_H
