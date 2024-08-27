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

//#include <Nuclex/Support/Events/Delegate.h> // for Delegate

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
      void *state,
      const ::OpusFileCallbacks *callbacks,
      const std::uint8_t *initialBytes = nullptr,
      std::size_t initialByteCount = 0
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_PLATFORM_OPUSAPI_H
