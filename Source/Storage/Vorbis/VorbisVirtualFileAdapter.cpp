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

#include "./VorbisVirtualFileAdapter.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include <Nuclex/Support/ScopeGuard.h> // for ScopeGuard

#include "Nuclex/Audio/Storage/VirtualFile.h" // for VitualFile

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to <see cref="byteCount" /> of data from a virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="target">Buffer that will receive the data read from the file</param>
  /// <param name="size">Size of an data record</param>
  /// <param name="membercount">Number of records that will be read</param>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>The number of bytes successfully read, or a negative value on error</returns>
  /// <remarks>
  ///   The vorbisfile library allows this function to read fewer bytes than requested,
  ///   except for zero, which is only allowed when the end of the file is reached.
  /// </remarks>
  template<typename TAdapterState>
  ::size_t vorbisReadBytes(void *target, ::size_t size, ::size_t memberCount, void *stateAsVoid) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);

    // Limit the read to the length of the virtual file (because our interface does
    // not allow leisurely reading past the end of the file and having the virtual file
    // 'clip' the read call by itself).
    std::size_t byteCountToRead;
    {
      std::uint64_t remainingByteCount = state.File->GetSize();

      if(state.FileCursor >= remainingByteCount) {
        return 0; // EOF!
      } else {
        byteCountToRead = size * memberCount;

        remainingByteCount -= state.FileCursor;
        if(remainingByteCount < static_cast<std::uint64_t>(byteCountToRead)) {
          byteCountToRead = static_cast<std::size_t>(remainingByteCount);
        }
      }
    }

    try {
      state.File->ReadAt(
        state.FileCursor, byteCountToRead, reinterpret_cast<std::uint8_t *>(target)
      );
    }
    catch(const std::exception &) {
      state.Error = std::current_exception();
      errno = EIO; // libvorbisfile checks errno to 
      return 0;
    }

    state.FileCursor += byteCountToRead;

    return byteCountToRead;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to a different position within the virtual file</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the seek is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <param name="offset">Offset relative to the specific anchor point</param>
  /// <param name="anchor">Anchor point, can be SEEK_SET, SEEK_CUR or SEEK_END</param>
  /// <returns>Zero on success, -1 on error or if seeking is not supported</returns>
  template<typename TAdapterState>
  int vorbisSeek(void *stateAsVoid, ::ogg_int64_t offset, int anchor) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);

    // fseek() on POSIX does in fact allow the file cursor to be placed beyond the end of
    // a file. We could emulate this behavior (by simply inserting zero bytes here or doing
    // some acrobatics for read-only streams), but let's, for now, go by the unproven
    // assumption that libvorbisfile will *not* place the file cursor beyond the file end.
    std::uint64_t newPosition = state.FileCursor;
    {
      std::uint64_t fileLength = state.File->GetSize();
      switch(anchor) {
        case SEEK_SET: {
          newPosition = static_cast<std::uint64_t>(offset);
          break;
        }
        case SEEK_CUR: {
          if(offset < 0) {
            if(state.FileCursor < static_cast<std::uint64_t>(-offset)) {
              assert(
                (state.FileCursor >= static_cast<std::uint64_t>(-offset)) &&
                u8"Seek from end stops at file start"
              );
              return -1;
            }
          }

          newPosition = state.FileCursor + offset;
          break;
        }
        case SEEK_END: {
          if(offset < 0) {
            if(fileLength < static_cast<std::uint64_t>(-offset)) {
              assert(
                (fileLength >= static_cast<std::uint64_t>(-offset)) &&
                u8"Seek from end stops at file start"
              );
              return -1;
            }
          }

          newPosition = fileLength + offset;
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

    state.FileCursor = newPosition;
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the current position of the file cursor in the virtual file</summary>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>
  ///   The absolute position of the file cursor or a negative value if seeking is not supported
  // </returns>
  long vorbisTell(void *stateAsVoid) {
    Nuclex::Audio::Storage::Vorbis::FileAdapterState &state = *reinterpret_cast<
      Nuclex::Audio::Storage::Vorbis::FileAdapterState *
    >(stateAsVoid);

    return static_cast<long>(state.FileCursor);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Closes the virtual file after the vorbisfile library is done with it</summary>
  /// <typeparam name="TAdapterState">
  ///   Type of state the close is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="stateAsVoid">State of the virtual file adapter class</param>
  /// <returns>Zero on success, EOF if the file could not be closed</returns>
  template<typename TAdapterState>
  int vorbisClose(void *stateAsVoid) {
    TAdapterState &state = *reinterpret_cast<TAdapterState *>(stateAsVoid);
    state.File.reset();

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<ReadOnlyFileAdapterState> FileAdapterFactory::CreateAdapterForReading(
    const std::shared_ptr<const VirtualFile> &readOnlyFile,
    ::ov_callbacks &fileCallbacks
  ) {
    std::unique_ptr<ReadOnlyFileAdapterState> adapter = (
      std::make_unique<ReadOnlyFileAdapterState>()
    );

    adapter->IsReadOnly = true;
    adapter->FileCursor = 0;
    adapter->Error = std::exception_ptr();
    adapter->File = readOnlyFile;

    fileCallbacks.read_func = &vorbisReadBytes<ReadOnlyFileAdapterState>;
    fileCallbacks.seek_func = &vorbisSeek<ReadOnlyFileAdapterState>;
    fileCallbacks.tell_func = &vorbisTell;
    fileCallbacks.close_func = &vorbisClose<ReadOnlyFileAdapterState>;

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

  std::unique_ptr<WritableFileAdapterState> FileAdapterFactory::CreateAdapterForWriting(
    const std::shared_ptr<VirtualFile> &writableFile,
    ::ov_callbacks &fileCallbacks
  ) {
    std::unique_ptr<WritableFileAdapterState> adapter = (
      std::make_unique<WritableFileAdapterState>()
    );

    adapter->IsReadOnly = false;
    adapter->FileCursor = 0;
    adapter->Error = std::exception_ptr();
    adapter->File = writableFile;

    fileCallbacks.read_func = &vorbisReadBytes<WritableFileAdapterState>;
    fileCallbacks.seek_func = &vorbisSeek<WritableFileAdapterState>;
    fileCallbacks.tell_func = &vorbisTell;
    fileCallbacks.close_func = &vorbisClose<WritableFileAdapterState>;

    return adapter;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
