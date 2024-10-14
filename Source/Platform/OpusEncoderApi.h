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

#ifndef NUCLEX_AUDIO_PLATFORM_OPUSENCODERAPI_H
#define NUCLEX_AUDIO_PLATFORM_OPUSENCODERAPI_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <string> // for std::string
#include <memory> // for std::shared_ptr
#include <vector> // for std::vector
#include <cstdint> // for std::uint32_t

#include <opusenc.h> // for Opus (or rather, the opusenc wrapper library)

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the opusenc API with error checking</summary>
  class OpusEncoderApi {

    /// <summary>Creates a comment record for an Opus audio file</summary>
    /// <remarks>The new comment record</remarks>
    public: static std::shared_ptr<::OggOpusComments> CreateComments();

    /// <summary>Creates an Opus audio file accessed through file callbacks</summary>
    /// <param name="state">State, will be passed unmodified to all file callbacks</param>
    /// <param name="callbacks">Callbacks through which file accesses will happen</param>
    /// <param name="sampleRate">Intended playback speed in samples per second</param>
    /// <param name="channelCount">Number of channels to be encoded</param>
    /// <param name="comments">An Ogg/Opus comment block with metadata to write</param>
    /// <returns>
    ///   A shared pointer to the Opus encoder which can be fed with audio data that
    ///   will then be encoded and writen via the specified write callbacks
    /// </returns>
    /// <remarks>
    ///   The returned shared pointer has a custom deleter set up, so this is fully RAII
    ///   compatible and once the pointer goes out of scope, the Opus encoder is destroyed.
    /// </remarks>
    public: static std::shared_ptr<::OggOpusEnc> CreateFromCallbacks(
      void *state,
      const ::OpusEncCallbacks *callbacks,
      std::size_t sampleRate,
      std::size_t channelCount,
      const std::shared_ptr<::OggOpusComments> comments = CreateComments()
    );

    /// <summary>Sends a control message with an integer arguments to the encoder</summary>
    /// <param name="encoder">Encoder to which the control message is being sent</param>
    /// <param name="request">Identifier of the control message</param>
    /// <param name="value">Integer value to pass along with the control message</param>
    public: static void ControlEncoder(
      const std::shared_ptr<::OggOpusEnc> &encoder, int request, int value
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_PLATFORM_OPUSENCODERAPI_H
