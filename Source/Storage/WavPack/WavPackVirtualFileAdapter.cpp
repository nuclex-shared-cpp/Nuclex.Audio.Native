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

#include "./WavPackVirtualFileAdapter.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include <Nuclex/Support/ScopeGuard.h> // for ScopeGuard

#include "Nuclex/Audio/Storage/VirtualFile.h" // for VitualFile

// Design notes:
//
// I'm not entirely happy with the lengths I have to go to adapt my VirtualFile interface
// for WavPack. Its use of ungetc() (push a character back into the read buffer so it is
// seen on the next read as if it was in the file) really complicates things.
//
// - It might suffice to allow for a single ungetc() character,
// - It might even suffice to just rewind the file cursor because libwavpack only ever
//   unreads bytes it has read immediately before (a comment in cli/utils.c says as much)
// - The fuzzer in fuzzing/fuzzer.cc also uses an ungetc buffer of exactly 1 byte.
//
// So we might get away with either a single byte buffer or just rewinding.
//
// But -- optimizing this adapter using special internal knowledge and building
// an implementation that only handles the usage patterns present in libwavpack would be
// rather dirty, and could theoretically set us up for failure with future libwavpack versions.
//
// And the impact is small, too. After scanning the file type and its header, libwavpack
// reads in decently-sized blocks. So that's why there's a complicated wavPackReadBytes()
// implementation that covers all potential usages, even those libwavpack doesn't need.
//

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to specified number of bytes from the file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer that will receive the data read</param>
  /// <param name="byteCount">Number of bytes that should be read at most</param>
  /// <returns>The number of bytes actually read</returns>
  template<typename TAdapterState>
  std::int32_t wavPackReadBytes(void *id, void *data, std::int32_t byteCount) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(id);
    assert(state.IsReadOnly && u8"File read is performed on read state");

    std::uint8_t *dataAsBytes = reinterpret_cast<std::uint8_t *>(data);

    // Determine the number of bytes we can read from the buffer. This buffer holds bytes
    // that have been pushed back into the stream by libwavpack (kinda awkward, but we need
    // to implement this to fully honor defined stream interface)
    std::size_t bytesFromBuffer;
    {
      std::size_t bufferedByteCount = state.BufferedBytes.size();
      bytesFromBuffer = std::min<std::size_t>(byteCount, bufferedByteCount);
    }

    // If the buffer had any data, begin by copying that (but don't remove it from the buffer
    // yet, if the VirtualFile read fails, we'll act as if the read never happened).
    if(bytesFromBuffer >= 1) {
      std::copy_n(
        state.BufferedBytes.data(),
        byteCount,
        dataAsBytes
      );
      if(bytesFromBuffer >= static_cast<std::size_t>(byteCount)) {
        state.BufferedBytes.erase(
          state.BufferedBytes.begin(),
          state.BufferedBytes.begin() + bytesFromBuffer
        );
        return byteCount;
      } else {
        dataAsBytes += bytesFromBuffer;
        byteCount -= static_cast<std::int32_t>(bytesFromBuffer);
      }
    }

    // Attempt to read up to the requested number of bytes from the virtual file
    std::size_t bytesFromFile;
    {
      std::uint64_t fileLength = state.File->GetSize();
      if(state.FileCursor >= fileLength) {
        bytesFromFile = 0;
      } else {
        fileLength -= state.FileCursor;
        bytesFromFile = static_cast<std::size_t>(
          std::min<std::uint64_t>(byteCount, fileLength)
        );
      }
    }
    if(bytesFromFile >= 1) {
      try {
        state.File->ReadAt(
          state.FileCursor,
          bytesFromFile,
          dataAsBytes
        );
      }
      catch(const std::exception &) {
        state.Error = std::current_exception();
        return 0; // error state, don't even try for a partial success
      }
    }

    // The virtual file read succeeded. Now we can update our state and shift
    // buffered bytes if we did take any.
    state.FileCursor += bytesFromFile;
    if(bytesFromBuffer >= 1) {
      state.BufferedBytes.erase(
        state.BufferedBytes.begin(),
        state.BufferedBytes.begin() + bytesFromBuffer
      );
      return static_cast<std::int32_t>(bytesFromBuffer + bytesFromFile);
    } else {
      return static_cast<std::int32_t>(bytesFromFile);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the specified number of bytes into the file</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer containing the data to be written</param>
  /// <param name="byteCount">Number of bytes that should be written</param>
  /// <returns>The number of bytes actually written</returns>
  std::int32_t wavPackWriteBytes(void *id, void *data, std::int32_t byteCount) {
    Nuclex::Audio::Storage::WavPack::WritableStreamAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::WritableStreamAdapterState *
    >(id);
    assert(!state.IsReadOnly && u8"File write is performed on write state");

    try {
      state.File->WriteAt(
        state.FileCursor,
        byteCount,
        reinterpret_cast<std::uint8_t *>(data)
      );
    }
    catch(const std::exception &) {
      state.Error = std::current_exception();
      return 0; // error state, don't even try for a partial success
    }

    // If bytes were pushed back into the read buffer, we leave them in there?
    // The libc docs for ungetc() only state that seeking discards the buffer.

    return byteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the current absolute position of the file cursor</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>The current absolute position of the file cursor in the file</returns>
  std::int64_t wavPackGetCurrentPosition(void *id) {
    Nuclex::Audio::Storage::WavPack::StreamAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::StreamAdapterState *
    >(id);

    return static_cast<std::int64_t>(state.FileCursor);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to the specified absolute position</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="position">Absolute position the file cursor should be placed at</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  template<typename TAdapterState>
  int wavPackSeekAbsolute(void *id, std::int64_t position) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(id);

    // fseek() on POSIX does in fact allow the file cursor to be placed beyond the end of
    // a file. We could emulate this behavior (by simply inserting zero bytes here or doing
    // some acrobatics for read-only streams), but let's, for now, go by the unproven
    // assumption that libwavpack will *not* place the file cursor beyond the end of the file.
    std::uint64_t fileLength = state.File->GetSize();
    if(fileLength < static_cast<std::uint64_t>(position)) {
      assert(
        (fileLength < static_cast<std::uint64_t>(position)) &&
        u8"Seek keeps file cursor within file boundaries"
      );
      return -1;
    }

    // As documented in the ungetc() function from libc, seeking should discard
    // the effects of ungetc()
    state.BufferedBytes.clear();

    state.FileCursor = static_cast<std::uint64_t>(position);
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor by specified offset</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="delta">Offset by which the file cursor should be placed at</param>
  /// <param name="anchor">Anchor relative to which the file cursor will be placed</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  template<typename TAdapterState>
  int wavPackSeekRelative(void *id, std::int64_t delta, int anchor) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(id);

    // Should invalid seeks become exceptions to the outside caller?
    //state.Error = std::make_exception_ptr(
    //  std::runtime_error(
    //    u8"Relative seek offset would place the file cursor before the start of the file"
    //  )
    //);

    // fseek() on POSIX does in fact allow the file cursor to be placed beyond the end of
    // a file. We could emulate this behavior (by simply inserting zero bytes here or doing
    // some acrobatics for read-only streams), but let's, for now, go by the unproven
    // assumption that libwavpack will *not* place the file cursor beyond the end of the file.
    std::uint64_t newPosition = state.FileCursor;
    {
      std::uint64_t fileLength = state.File->GetSize();
      switch(anchor) {
        case SEEK_SET: {
          newPosition = static_cast<std::uint64_t>(delta);
          break;
        }
        case SEEK_CUR: {
          if(delta < 0) {
            if(state.FileCursor < static_cast<std::uint64_t>(-delta)) {
              assert(
                (state.FileCursor >= static_cast<std::uint64_t>(-delta)) &&
                u8"Seek from end stops at file start"
              );
              return -1;
            }
          }

          newPosition = state.FileCursor + delta;
          break;
        }
        case SEEK_END: {
          if(delta < 0) {
            if(fileLength < static_cast<std::uint64_t>(-delta)) {
              assert(
                (fileLength >= static_cast<std::uint64_t>(-delta)) &&
                u8"Seek from end stops at file start"
              );
              return -1;
            }
          }

          newPosition = fileLength + delta;
          break;
        }
        default: {
          return -1;
        }
      }

      if(fileLength < newPosition) {
        assert((fileLength < newPosition) && u8"Seek keeps file cursor within file boundaries");
        return -1;
      }
    }

    // As documented in the ungetc() function from libc, seeking should discard
    // the effects of ungetc()
    state.BufferedBytes.clear();

    state.FileCursor = newPosition;
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Buffers a single byte so it can be read as if it was in the stream</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="c">Byte that will be appended to the stream</param>
  /// <returns>The buffered byte</returns>
  /// <remarks>
  ///   This method is incredibly awkward. We have to buffer this (and possibly multiple)
  ///   bytes to be delivered as if they were read from the input stream, but the method
  ///   this is based on (<code>ungetc()</code>) is also documented to indicate that
  ///   seeking will discard any artificially buffered bytes.
  /// </remarks>
  int wavPackBufferByte(void *id, int c) {
    Nuclex::Audio::Storage::WavPack::StreamAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::StreamAdapterState *
    >(id);

    if(c >= 256) {
      assert((c < 256) && u8"ungetc() isn't used to buffer an EOF or other value");
      return -1;
    }

    state.BufferedBytes.push_back(static_cast<std::uint8_t>(c));

    return c;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simulates a single byte as if it was in the input stream</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>The length of the input stream in bytes</returns>
  template<typename TAdapterState>
  std::int64_t wavPackGetLength(void *id) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(id);
    return static_cast<std::int64_t>(state.File->GetSize());
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the input stream supports seeking</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero is the file is not seekable, any other value if it is</returns>
  int wavPackIsSeekable(void *id) {
    (void)id;

    // While the VirtualFile interface can be used for sequential data (by enforcing
    // usage from a single thread and monotonic ReadAt() positions), that is a usage
    // pattern or us, not a queryable property of the VirtualFile interface...
    return 1; // just claim that seeking is supported
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Truncates the length fo the input file to the current file cursor</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero on success, -1 in case of an error</returns>
  int wavPackTruncateToFileCursor(void *id) {
    (void)id;

    assert(u8"TruncateToFileCursor() is never called");
    return -1;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Flushes any unwritten data in internal buffers and closes the file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero on success, EOF in case of an error</returns>
  template<typename TAdapterState>
  int wavPackClose(void *id) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(id);
    state.File.reset();

    #if 0 // Perhaps it's better to handle this ourselves.
    TAdapterState *state = reinterpret_cast<TAdapterState *>(id);
    delete state; // This is under the derived type and will release the shared_ptr
    #endif

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<ReadOnlyStreamAdapterState> StreamAdapterFactory::CreateAdapterForReading(
    const std::shared_ptr<const VirtualFile> &readOnlyFile,
    WavpackStreamReader64 &streamReader
  ) {
    std::unique_ptr<ReadOnlyStreamAdapterState> adapter = (
      std::make_unique<ReadOnlyStreamAdapterState>()
    );

    adapter->IsReadOnly = true;
    adapter->FileCursor = 0;
    //adapter->BufferedBytes.clear();
    adapter->Error = std::exception_ptr();
    adapter->File = readOnlyFile;

    streamReader.read_bytes = &wavPackReadBytes<ReadOnlyStreamAdapterState>;
    streamReader.write_bytes = &wavPackWriteBytes;
    streamReader.get_pos = &wavPackGetCurrentPosition;
    streamReader.set_pos_abs = &wavPackSeekAbsolute<ReadOnlyStreamAdapterState>;
    streamReader.set_pos_rel = &wavPackSeekRelative<ReadOnlyStreamAdapterState>;
    streamReader.push_back_byte = &wavPackBufferByte;
    streamReader.get_length = &wavPackGetLength<ReadOnlyStreamAdapterState>;
    streamReader.can_seek = &wavPackIsSeekable;
    streamReader.truncate_here = &wavPackTruncateToFileCursor;
    streamReader.close = &wavPackClose<ReadOnlyStreamAdapterState>;

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<WritableStreamAdapterState> StreamAdapterFactory::CreateAdapterForWriting(
    const std::shared_ptr<VirtualFile> &writableFile,
    WavpackStreamReader64 &streamReader
  ) {
    std::unique_ptr<WritableStreamAdapterState> adapter = (
      std::make_unique<WritableStreamAdapterState>()
    );

    adapter->IsReadOnly = false;
    adapter->FileCursor = 0;
    //adapter->BufferedBytes.clear();
    adapter->Error = std::exception_ptr();
    adapter->File = writableFile;

    streamReader.read_bytes = &wavPackReadBytes<WritableStreamAdapterState>;
    streamReader.write_bytes = &wavPackWriteBytes;
    streamReader.get_pos = &wavPackGetCurrentPosition;
    streamReader.set_pos_abs = &wavPackSeekAbsolute<WritableStreamAdapterState>;
    streamReader.set_pos_rel = &wavPackSeekRelative<WritableStreamAdapterState>;
    streamReader.push_back_byte = &wavPackBufferByte;
    streamReader.get_length = &wavPackGetLength<WritableStreamAdapterState>;
    streamReader.can_seek = &wavPackIsSeekable;
    streamReader.truncate_here = &wavPackTruncateToFileCursor;
    streamReader.close = &wavPackClose<WritableStreamAdapterState>;

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

  void StreamAdapterState::RethrowPotentialException(StreamAdapterState &streamAdapterState) {
    if(static_cast<bool>(streamAdapterState.Error)) {
      ON_SCOPE_EXIT {
        streamAdapterState.Error = nullptr;
      };
      std::rethrow_exception(streamAdapterState.Error);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
