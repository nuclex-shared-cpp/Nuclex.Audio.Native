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

#ifndef NUCLEX_AUDIO_PLATFORM_FLACAPI_H
#define NUCLEX_AUDIO_PLATFORM_FLACAPI_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <memory> // for std::shared_ptr
#include <cstdint> // for std::int32_t, std::uint64_t

#include <FLAC/stream_decoder.h> // for the plain C FLAC decoder

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the FLAC API with error checking</summary>
  class FlacApi {

    /// <summary>Creates a new FLAC stream decoder</summary>
    /// <returns>The new FLAC stream decoder</returns>
    public: static std::shared_ptr<::FLAC__StreamDecoder> NewStreamDecoder();

    /// <summary>Enables or disables MD5 checking for decoded audio data</summary>
    /// <param name="decoder">Decoder on which MD5 checking will be enabled or disabled</param>
    /// <param name="enable">True to enable MD5 checking, false to disable it</param>
    public: static void EnableMd5Checking(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder, bool enable = true
    );

    /// <summary>Opens a plain file to decode as a FLAC audio file</summary>
    /// <param name="decoder">Decoder in which the file will be opened</param>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="writeCallback">Callback that receives decoded audio data</param>
    /// <param name="metadataCallback">Callback that receives the embedded metadata</param>
    /// <param name="errorCallback">Callback that will be invoked on decoding errors</param>
    /// <param name="clientData">Pointer that will be passed unchaged to callbacks</param>
    public: static void InitFile(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      const char *path,
      ::FLAC__StreamDecoderWriteCallback writeCallback,
      ::FLAC__StreamDecoderMetadataCallback metadataCallback,
      ::FLAC__StreamDecoderErrorCallback errorCallback,
      void *clientData
    );

    /// <summary>Opens a stream accessed via callbacks as a FLAC audio file</summary>
    /// <param name="rootCauseException">
    ///   Should receive any exception that happened in the virtual file and will be thrown
    ///   instead of a generic FLAC error if it becomes filled during the FLAC API call
    /// </param>
    /// <param name="decoder">Decoder in which the stream will be opened</param>
    /// <param name="readCallback">Callback used to read data from the stream</param>
    /// <param name="seekCallback">Callback to move the file cursor in the stream</param>
    /// <param name="tellCallback">Callback that returns the position of the file cursor</param>
    /// <param name="lengthCallback">Callback that returns the stream's length</param>
    /// <param name="eofCallback">Callback that checks if the file cursor is at the end</param>
    /// <param name="writeCallback">Callback that receives decoded audio data</param>
    /// <param name="metadataCallback">Callback that receives the embedded metadata</param>
    /// <param name="errorCallback">Callback that will be invoked on decoding errors</param>
    /// <param name="clientData">Pointer that will be passed unchaged to callbacks</param>
    public: static void InitStream(
      const std::exception_ptr &rootCauseException,
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      ::FLAC__StreamDecoderReadCallback readCallback,
      ::FLAC__StreamDecoderSeekCallback seekCallback,
      ::FLAC__StreamDecoderTellCallback tellCallback,
      ::FLAC__StreamDecoderLengthCallback lengthCallback,
      ::FLAC__StreamDecoderEofCallback eofCallback,
      ::FLAC__StreamDecoderWriteCallback writeCallback,
      ::FLAC__StreamDecoderMetadataCallback metadataCallback,
      ::FLAC__StreamDecoderErrorCallback errorCallback,
      void *clientData
    );

    /// <summary>Shuts a stream decoder down after decoding has finished</summary>
    /// <param name="decoder">Stream decoder that will be shut down</param>
    /// <remarks>
    ///   The FLAC API documentation recommends calling this as the counterpart to every
    ///   call to Init (InitFile(), InitStream() in this wrapper).
    /// </remarks>
    public: static void Finish(const std::shared_ptr<::FLAC__StreamDecoder> &decoder);

    /// <summary>Enables the metadata callback for the specified metadata block type</summary>
    /// <param name="deocder">Decoder for which a metadata callback will be enabled</param>
    /// <param name="metadataType">Metadata block type for which to enable the callback</param>
    public: static void SetRespondMetadata(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      ::FLAC__MetadataType metadataType
    );

    /// <summary>Processes a single block from the opened FLAC audio file</summary>
    /// <param name="decoder">Decoder that will process a single block</param>
    /// <returns>The callback return value or true if everything was processed</returns>
    public: static bool ProcessSingle(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder
    );

    /// <summary>Processes all blocks from the opened FLAC audio file</summary>
    /// <param name="decoder">Decoder that will process all of the file's blocks</param>
    /// <returns>True if the end was reached, false if a callback returned false</returns>
    public: static bool ProcessUntilEndOfStream(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder
    );

    /// <summary>Seeks to the specified frame</summary>
    /// <param name="decoder">Decoder that will seek to the specified frame</param>
    /// <param name="frameIndex">Index of the frame the decoder will seek to</param>
    public: static void SeekAbsolute(
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      std::uint64_t frameIndex
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_PLATFORM_FLACAPI_H
