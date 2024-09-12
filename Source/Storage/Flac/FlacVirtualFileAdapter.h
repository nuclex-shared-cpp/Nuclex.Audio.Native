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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACVIRTUALFILEADAPTER_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACVIRTUALFILEADAPTER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t
#include <vector> // for std::vector

#include <FLAC/stream_decoder.h> // for the callback signatures

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Processes the output of the FLAC stream decodeR</summary>
  class FlacDecodeProcessor {

    /// <summary>Frees all memory used by the processor</summary>
    public: virtual ~FlacDecodeProcessor() = default;

    /// <summary>Called to process an audio frame after it has been decoded</summary>
    /// <param name="frame">Informations about the decoded audio frame</param>
    /// <param name="buffer">Stores the decoded audio samples</param>
    /// <returns>True to continue decoding, false to stop at this point</returns>
    public: virtual bool ProcessAudioFrame(
      const ::FLAC__Frame *frame,
      const ::FLAC__int32 *const buffer[]
    ) = 0;

    /// <summary>Called to process any metadata encountered in the FLAC file</summary>
    /// <param name="metadata">Metadata the FLAC stream decoder has encountered</param>
    public: virtual void ProcessMetadata(
      const ::FLAC__StreamMetadata *metadata
    ) = 0;

    /// <summary>Called to provide a detailed status when a decoding error occurs</summary>
    /// <param name="status">Error status of the stream decoder</param>
    public: virtual void HandleError(
      ::FLAC__StreamDecoderErrorStatus status
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream adapters</summary>
  struct FileAdapterState {

    /// <summary>
    ///   Re-throws any exceptions that happened while libopsufile accessed the adapter
    /// </summary>
    /// <param name="streamAdapterState">State that will be checked for an exception</param>
    public: static void RethrowPotentialException(FileAdapterState &state);

    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsReadOnly;
    /// <summary>Current position of the emulated file cursor</summary>
    public: std::uint64_t FileCursor;
    /// <summary>Receives decoded samples, metadata and error reports</summary>
    public: FlacDecodeProcessor *DecodeProcessor;
    /// <summary>Stores any exception thrown by the virtual file interface</summary>
    public: std::exception_ptr Error;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream reader adapter</summary>
  struct ReadOnlyFileAdapterState : public FileAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<const VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream writer adapter</summary>
  struct WritableFileAdapterState : public FileAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Initializes FLAC stream decoders with virtual file callbacks</summary>
  class FileAdapterFactory {

    /// <summayr>Initialies a FLAC stream deocder with callbacks for a read-only file</summary>
    /// <param name="readOnlyFile">Virtual file the adapter will read from</param>
    /// <param name="decoder">
    ///   FLAC stream decoder that will set up to use the adapter
    /// </param>
    /// <param name="decodeProcessor">Interface to deliver decoded data to</param>
    /// <returns>
    ///   A state that needs to be kept alive for as long as the stream decoder exists
    /// </returns>
    public: static std::unique_ptr<ReadOnlyFileAdapterState> InitStreamDecoderForReading(
      const std::shared_ptr<const VirtualFile> &readOnlyFile,
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      FlacDecodeProcessor *decodeProcessor
    );

    /// <summayr>Initialies a FLAC stream deocder with callbacks for a writable file</summary>
    /// <param name="writableFile">Virtual file the adapter will write to</param>
    /// <param name="fileCallbacks">
    ///   FLAC stream decoder that will set up to use the adapter
    /// </param>
    /// <param name="decodeProcessor">Interface to deliver decoded data to</param>
    /// <returns>
    ///   A state that needs to be kept alive for as long as the stream decoder exists
    /// </returns>
    public: static std::unique_ptr<WritableFileAdapterState> InitStreamDecoderForWriting(
      const std::shared_ptr<VirtualFile> &writableFile,
      const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
      FlacDecodeProcessor *decodeProcessor
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACVIRTUALFILEADAPTER_H
