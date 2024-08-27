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

#ifndef NUCLEX_AUDIO_PLATFORM_OPUSAPI_H
#define NUCLEX_AUDIO_PLATFORM_OPUSAPI_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <Nuclex/Support/Events/Delegate.h> // for Delegate

#include <string> // for std::string
#include <memory> // for std::shared_ptr
#include <vector> // for std::vector
#include <cstdint> // for std::uint32_t

#include <opusfile.h> // for Opus (or rather, the opusfile wrapper library)

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Opus / opusfile API with error checking</summary>
  class OpusApi {

    /// <summary>Opens an Opus audio file accessed through file callbacks</summary>
    /// <param name="state">State, will be passed unmodified to all file callbacks</param>
    /// <param name="callbacks">Callbacks through which file accesses will happen</param>
    /// <param name="initialBytes">Extra buffer of bytes that have alraedy been read</param>
    /// <param name="initialByteCount">Number of bytes in the extra buffere</param>
    /// <returns>
    ///   A shared pointer to the opened opus file which can be used with other functions
    ///   provided by the libopusfile library.
    /// </returns>
    /// <remarks>
    ///   The returned shared pointer has a custom deleter set up, so this is fully RAII
    ///   compatible and once the pointer goes out of scope, the WavPack context is
    ///   closed again.
    /// </remarks>
    public: static std::shared_ptr<::OggOpusFile> OpenFromCallbacks(
      const Nuclex::Support::Events::Delegate<void()> &throwRootCauseException,
      void *state,
      const ::OpusFileCallbacks *callbacks,
      const std::uint8_t *initialBytes = nullptr,
      std::size_t initialByteCount = 0
    );

    /// <summary>Retrieves header information describing an audio track</summary>
    /// <param name="opusFile">
    ///   Opened Opus audio file to retrieve header information for
    /// </param>
    /// <param name="linkIndex">
    ///   Index of the link whose channels to count, -1 for the current link.
    ///   These are *not* the same as tracks in a Matroska or MP4 container,
    ///   links are used to chain audio streams sequentially, not interleave them.
    /// </param>
    /// <returns>An OpusHead structure with metadata about the Opus audio file</returns>
    /// <remarks>
    ///   The returned OpusHead structure is part of the internal libopus plumbing,
    ///   so it will stay valid, at least until other libopus operations are performed.
    /// </remarks>
    public: static const OpusHead &GetHeader(
      const std::shared_ptr<::OggOpusFile> &opusFile, int linkIndex = -1
    );

    /// <summary>Counts the number of links in the OGG contianer</summary>
    /// <param name="opusFile">Opened Opus audio file to count the links in</param>
    /// <returns>The number of links on the OGG container</returns>
    public: static std::size_t CountLinks(const std::shared_ptr<::OggOpusFile> &opusFile);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_PLATFORM_OPUSAPI_H
