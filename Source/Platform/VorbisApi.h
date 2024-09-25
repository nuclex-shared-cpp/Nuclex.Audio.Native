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

#ifndef NUCLEX_AUDIO_PLATFORM_VORBISAPI_H
#define NUCLEX_AUDIO_PLATFORM_VORBISAPI_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include <string> // for std::string
#include <memory> // for std::shared_ptr
#include <vector> // for std::vector
#include <cstdint> // for std::uint32_t

#include <vorbis/vorbisfile.h> // for Vorbis (or rather, the vorbisfile wrapper library)

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Vorbis / vorbisfile API with error checking</summary>
  class VorbisApi {

    /// <summary>Opens a Vorbis audio file accessed through file callbacks</summary>
    /// <param name="rootCauseException">
    ///   Captured exception pointer that will be rethrown in place of the libvorbisfile error
    ///   if something goes wrong. This allows proper error reporting if a fault occurrs
    ///   in a callback that is invoked by libvorbisfile.
    /// </param>
    /// <param name="state">State, will be passed unmodified to all file callbacks</param>
    /// <param name="callbacks">Callbacks through which file accesses will happen</param>
    /// <param name="initialBytes">Extra buffer of bytes that have alraedy been read</param>
    /// <param name="initialByteCount">Number of bytes in the extra buffere</param>
    /// <returns>
    ///   A shared pointer to the opened Vorbis file which can be used with other functions
    ///   provided by the libvorbisfile library.
    /// </returns>
    /// <remarks>
    ///   The returned shared pointer has a custom deleter set up, so this is fully RAII
    ///   compatible and once the pointer goes out of scope, the Ogg Vorbis file is
    ///   closed again.
    /// </remarks>
    public: static std::shared_ptr<::OggVorbis_File> OpenFromCallbacks(
      const std::exception_ptr &rootCauseException,
      void *state,
      const ::ov_callbacks &callbacks,
      const char *initialBytes = nullptr,
      long initialByteCount = 0
    );

    /// <summary>Counts the number of streams in the OGG contianer</summary>
    /// <param name="vorbisFile">Opened Vorbis audio file to count the streams in</param>
    /// <returns>The number of streams on the OGG container</returns>
    public: static std::size_t CountStreams(const std::shared_ptr<::OggVorbis_File> &vorbisFile);

    /// <summary>Looks up the stream information structure in a Vorbis file</summary>
    /// <param name="vorbisFile">
    ///   Opened Vorbis file to retrieve the stream information from
    /// </param>
    /// <param name="streamIndex">Index of the stream whose information to retrieve</param>
    /// <returns>The stream information structure of the specified file and stream</returns>
    public: static const ::vorbis_info &GetStreamInformation(
      const std::shared_ptr<::OggVorbis_File> &vorbisFile,
      int streamIndex = -1
    );

    /// <summary>Looks up the comment structure in a Vorbis file</summary>
    /// <param name="vorbisFile">Opened Vorbis file to retrieve the comments from</param>
    /// <param name="streamIndex">Index of the stream whose comments to retrieve</param>
    /// <returns>The comments for the specified file and stream</returns>
    public: static const ::vorbis_comment &GetComments(
      const std::shared_ptr<::OggVorbis_File> &vorbisFile,
      int streamIndex = -1
    );

    /// <summary>Looks up the total number of audio frames in a Vorbis file</summary>
    /// <param name="vorbisFile">Opened Vorbis file whose audio frames to count</param>
    /// <param name="streamIndex">Index of the stream whose samples to count</param>
    /// <returns>The number of audio samples (per channel?) in the specified file</returns>
    public: static std::uint64_t CountPcmSamples(
      const std::shared_ptr<::OggVorbis_File> &vorbisFile,
      int streamIndex = -1
    );

    /// <summary>Seeks to the specified sample in a Vorbis audio stream</summary>
    /// <param name="rootCauseException">
    ///   Captured exception pointer that will be rethrown in place of the libvorbisfile error
    ///   if something goes wrong. This allows proper error reporting if a fault occurrs
    ///   in a callback that is invoked by libvorbisfile.
    /// </param>
    /// <param name="vorbisFile">Opened Vorbis file in which to seek</param>
    /// <param name="sampleIndex">Index of the sample to which to seek</param>
    /// <returns>The number of audio samples (per channel?) in the specified file</returns>
    public: static void Seek(
      const std::exception_ptr &rootCauseException,
      const std::shared_ptr<::OggVorbis_File> &vorbisFile,
      ::ogg_int64_t sampleIndex
    );

    /// <summary>Decodes a block of samples from the Vorbis audio file</summary>
    /// <param name="rootCauseException">
    ///   Captured exception pointer that will be rethrown in place of the libvorbisfile error
    ///   if something goes wrong. This allows proper error reporting if a fault occurrs
    ///   in a callback that is invoked by libvorbisfile.
    /// </param>
    /// <param name="vorbisFile">Opened Vorbis file in which to seek</param>
    /// <param name="channelBuffers">
    ///   Receives pointers to buffers holding the decoded audio channels
    /// </param>
    /// <param name="sampleCount">Number of samples that should be decoded</param>
    /// <param name="outStreamIndex">
    ///   Recevies the index of the stream that is being decoded
    /// </param>
    /// <returns>The number of samples that have been decoded</returns>
    public: static std::size_t ReadFloat(
      const std::exception_ptr &rootCauseException,
      const std::shared_ptr<::OggVorbis_File> &vorbisFile,
      float **&channelBuffers,
      int sampleCount,
      int &outStreamIndex
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)

#endif // NUCLEX_AUDIO_PLATFORM_VORBISAPI_H
