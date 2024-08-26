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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSDETECTION_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSDETECTION_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid OPUS file possible</summary>
  /// <remarks>
  ///   Unknown. The smallest file I can produce with opusenc is 470 bytes, but I suspect
  ///   that, armed with a hex editor and a deep understanding of OGG+OPUS, one could
  ///   create a much smaller example. The documentation says 57 bytes are needed for
  ///   a good chance at detecting OPUS, better yet 512 bytes.
  /// </remarks>
  constexpr const std::size_t SmallestPossibleOpusSize = 57; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading OPUS files using libopus</summary>
  class Detection {

    /// <summary>Checks if the specified file extension indicates an .opus file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates an .opus file</returns>
    public: static bool DoesFileExtensionSayOpus(const std::string &extension);

    /// <summary>Checks if the specified file extension indicates an .ogg file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates an .ogg file</returns>
    public: static bool DoesFileExtensionSayOgg(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid OPUS header</summary>
    /// <param name="source">File that will be checked for a valid OPUS header</param>
    /// <returns>True if a valid OPUS header was found, false otherwise</returns>
    public: static bool CheckIfOpusHeaderPresent(const VirtualFile &source);

    /// <summary>Checks if the specified file starts with a valid OPUS header</summary>
    /// <param name="source">File that will be checked for a valid OPUS header</param>
    /// <returns>True if a valid OPUS header was found, false otherwise</returns>
    public: static bool CheckIfOpusHeaderPresentLite(const VirtualFile &source);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSDETECTION_H
