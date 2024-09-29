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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "./FlacVirtualFileAdapter.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include <Nuclex/Support/ScopeGuard.h> // for ScopeGuard

#include "Nuclex/Audio/Storage/VirtualFile.h" // for VitualFile

#include "../../Platform/FlacApi.h" // for the wrapped FLAC API methods

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to <see cref="bytes" /> of data from a virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="decoder">FLAC stream decoder that is requesting the bytes</param>
  /// <param name="buffer">Buffer to store the data</param>
  /// <param name="bytes">Maximum number of bytes to read, receives actual bytes read</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Whether the bytes were read successfully, failed or data ran out</returns>
  template<typename TAdapterState>
  ::FLAC__StreamDecoderReadStatus flacRead(
    const ::FLAC__StreamDecoder *decoder,
    ::FLAC__byte buffer[], size_t *bytes,
    void *stateAsVoid
  ) {
    (void)decoder;
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);

    // Limit the read to the length of the virtual file (because our interface does
    // not allow leisurely reading past the end of the file and having the virtual file
    // 'clip' the read call by itself).
    std::size_t byteCount = *bytes;
    {
      std::uint64_t fileLength = state.File->GetSize();
      if(state.FileCursor >= fileLength) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
      } else {
        fileLength -= state.FileCursor;
        if(fileLength < static_cast<std::uint64_t>(byteCount)) {
          *bytes = byteCount = static_cast<std::size_t>(fileLength);
        }
      }
    }
    if(byteCount == 0) {
      return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    try {
      state.File->ReadAt(state.FileCursor, byteCount, reinterpret_cast<std::byte *>(buffer));
    }
    catch(const std::exception &) {
      state.Error = std::current_exception();
      return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    state.FileCursor += byteCount;

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to a different position within the virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the seek is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="decoder">FLAC stream decoder that is requesting the seek</param>
  /// <param name="newPosition">Absolute file offset to seek to</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Whether the seek was performed or failed</returns>
  template<typename TAdapterState>
  ::FLAC__StreamDecoderSeekStatus flacSeek(
    const ::FLAC__StreamDecoder *decoder,
    ::FLAC__uint64 newPosition,
    void *stateAsVoid
  ) {
    (void)decoder;
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);

    std::uint64_t fileLength = state.File->GetSize();
    if(fileLength < newPosition) {
      assert(
        (fileLength < newPosition) && u8"Seek keeps file cursor within file boundaries"
      );
      return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }

    state.FileCursor = newPosition;

    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the current position of the file cursor in the virtual file</summary>
  /// <param name="decoder">FLAC stream decoder that is requesting the file position</param>
  /// <param name="currentPosition">Receives the current position of the file cursor</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Whether the file cursor was retrieved successfully</returns>
  ::FLAC__StreamDecoderTellStatus flacTell(
    const ::FLAC__StreamDecoder *decoder,
    ::FLAC__uint64 *currentPosition,
    void *stateAsVoid
  ) {
    (void)decoder;
    Nuclex::Audio::Storage::Flac::FileAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::Flac::FileAdapterState *
    >(stateAsVoid);

    *currentPosition = state.FileCursor;

    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the length of the virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the length is queried from (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="decoder">FLAC stream decoder that is requesting the file position</param>
  /// <param name="streamLength">Receives the length of the virtual file</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Whether the length was successfully retrieved</returns>
  template<typename TAdapterState>
  ::FLAC__StreamDecoderLengthStatus flacLength(
    const ::FLAC__StreamDecoder *decoder,
    ::FLAC__uint64 *streamLength,
    void *stateAsVoid
  ) {
    (void)decoder;
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);

    *streamLength = state.File->GetSize();

    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the file cursor is at the end of the virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the eof check is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="decoder">FLAC stream decoder that is requesting the eof state</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>True if the file cursor is at the end of the virtual file</returns>
  template<typename TAdapterState>
  ::FLAC__bool flacEof(
    const ::FLAC__StreamDecoder *decoder,
    void *stateAsVoid
  ) {
    (void)decoder;
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);
    return (state.FileCursor >= state.File->GetSize()) ? 1 : 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Forwards the sample processing callback to the decode processor</summary>
  /// <param name="decoder">FLAC stream decoder that has decoded audio samples</param>
  /// <param name="frame">Informations about the decoded audio frame</param>
  /// <param name="buffer">Set of buffers that hold the decoded audio samples</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Whether the decoding should continue or be aborted</returns>
  ::FLAC__StreamDecoderWriteStatus flacProcessSamples(
    const ::FLAC__StreamDecoder *decoder,
    const ::FLAC__Frame *frame,
    const ::FLAC__int32 *const buffer[],
    void *stateAsVoid
  ) {
    (void)decoder;
    Nuclex::Audio::Storage::Flac::FileAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::Flac::FileAdapterState *
    >(stateAsVoid);

    bool shouldContinue;
    try {
      shouldContinue = state.DecodeProcessor->ProcessAudioFrame(*frame, buffer);
    }
    catch(const std::exception &) {
      state.Error = std::current_exception();
      shouldContinue = false;
    }

    if(shouldContinue) {
      return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    } else {
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Forwards the metadata reporting callback to the decode processor</summary>
  /// <param name="decoder">FLAC stream decoder that is has encountered metadata</param>
  /// <param name="metadata">Metadata that has been encountered by the decoder</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  void flacProcessMetadata(
    const ::FLAC__StreamDecoder *decoder,
    const ::FLAC__StreamMetadata *metadata,
    void *stateAsVoid
  ) {
    (void)decoder;
    Nuclex::Audio::Storage::Flac::FileAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::Flac::FileAdapterState *
    >(stateAsVoid);

    // Currently, the ProcessMetadata() method is declared 'noexcept'. We could catch
    // exceptions here, set the 'Error' exception_ptr, then fail on the next processSamples()
    // callback and finally re-throw the exception when the decode call fails.
    //
    // ...but why go to these lengths? libflac obviously doesn't expect this method to fail
    // (it returns void) and the metadata recipient is always this library, not user-provided
    // code, so we can guarantee to abide by the 'noexcept' requirement.
    state.DecodeProcessor->ProcessMetadata(*metadata);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Forwards the error reporting callback to the decode processor</summary>
  /// <param name="decoder">FLAC stream decoder that is has encountered an error</param>
  /// <param name="status">Error status of the FLAC decoder</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  void flacHandleError(
    const ::FLAC__StreamDecoder *decoder,
    ::FLAC__StreamDecoderErrorStatus status,
    void *stateAsVoid
  ) {
    (void)decoder;
    Nuclex::Audio::Storage::Flac::FileAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::Flac::FileAdapterState *
    >(stateAsVoid);

    state.DecodeProcessor->HandleError(status);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<ReadOnlyFileAdapterState> FileAdapterFactory::InitStreamDecoderForReading(
    const std::shared_ptr<const VirtualFile> &readOnlyFile,
    const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
    FlacDecodeProcessor *decodeProcessor
  ) {
    std::unique_ptr<ReadOnlyFileAdapterState> adapter = (
      std::make_unique<ReadOnlyFileAdapterState>()
    );

    adapter->IsReadOnly = true;
    adapter->FileCursor = 0;
    adapter->DecodeProcessor = decodeProcessor;
    adapter->Error = std::exception_ptr();
    adapter->File = readOnlyFile;

    Platform::FlacApi::InitStream(
      adapter->Error,
      decoder,
      &flacRead<ReadOnlyFileAdapterState>,
      &flacSeek<ReadOnlyFileAdapterState>,
      &flacTell,
      &flacLength<ReadOnlyFileAdapterState>,
      &flacEof<ReadOnlyFileAdapterState>,
      &flacProcessSamples,
      &flacProcessMetadata,
      &flacHandleError,
      adapter.get()
    );

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<WritableFileAdapterState> FileAdapterFactory::InitStreamDecoderForWriting(
    const std::shared_ptr<VirtualFile> &writableFile,
    const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
    FlacDecodeProcessor *decodeProcessor
  ) {
    std::unique_ptr<WritableFileAdapterState> adapter = (
      std::make_unique<WritableFileAdapterState>()
    );

    // This method doesn't make much sense as it is.
    //
    // For consistency and easy-of-understand, I'm implementing it to follow the other
    // codec implementations. Howver, for FLAC, the decoder and encoder are split into
    // two unrealted classes (and the decoder has, in fact, no write callbacks it could
    // invoke), so once I implement encoding, this will need several other callbacks and
    // a FLAC__StreamEncoder or similar to open a stream on.

    adapter->IsReadOnly = true;
    adapter->FileCursor = 0;
    adapter->DecodeProcessor = decodeProcessor;
    adapter->Error = std::exception_ptr();
    adapter->File = writableFile;

    Platform::FlacApi::InitStream(
      adapter->Error,
      decoder,
      &flacRead<WritableFileAdapterState>,
      &flacSeek<WritableFileAdapterState>,
      &flacTell,
      &flacLength<WritableFileAdapterState>,
      &flacEof<WritableFileAdapterState>,
      &flacProcessSamples,
      &flacProcessMetadata,
      &flacHandleError,
      adapter.get()
    );

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
