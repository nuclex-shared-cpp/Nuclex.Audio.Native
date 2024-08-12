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

#include "./WavPackHelpers.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to specified number of bytes from the file</summary>
  /// <typeparam name="TReadOrWriteEnvironment">
  ///   Type of environment the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer that will receive the data read</param>
  /// <param name="byteCount">Number of bytes that should be read at most</param>
  /// <returns>The number of bytes actually read</returns>
  template<typename TReadOrWriteEnvironment>
  std::int32_t wavPackReadBytes(void *id, void *data, std::int32_t byteCount) {
    TReadOrWriteEnvironment &environment = *reinterpret_cast<
      TReadOrWriteEnvironment *
    >(id);
    assert(environment.IsReadOnly && u8"File read is performed on read environment");

    std::uint8_t *dataAsBytes = reinterpret_cast<std::uint8_t *>(data);

    // Determine the number of bytes we can read from the buffer. This buffer holds bytes
    // that have been pushed back into the stream by libwavpack (kinda awkward, but we need
    // to implement this to fully honor defined stream interface)
    std::size_t bytesFromBuffer;
    {
      std::size_t bufferedByteCount = environment.BufferedBytes.size();
      bytesFromBuffer = std::min<std::size_t>(byteCount, bufferedByteCount);
    }

    // If the buffer had any data, begin by copying that (but don't remove it from the buffer
    // yet, if the VirtualFile read fails, we'll act as if the read never happened).
    if(bytesFromBuffer >= 1) {
      std::copy_n(
        environment.BufferedBytes.data(),
        byteCount,
        dataAsBytes
      );
      if(bytesFromBuffer >= byteCount) {
        environment.BufferedBytes.erase(
          environment.BufferedBytes.begin(),
          environment.BufferedBytes.begin() + byteCount
        );
        return byteCount;
      } else {
        dataAsBytes += bytesFromBuffer;
        byteCount -= byteCount;
      }
    }

    // Attempt to read up to the requested number of bytes from the virtual file
    std::size_t bytesFromFile;
    {
      std::uint64_t fileLength = environment.File->GetSize();
      bytesFromFile = static_cast<std::size_t>(std::min<std::uint64_t>(byteCount, fileLength));
    }
    if(bytesFromFile >= 1) {
      try {
        environment.File->ReadAt(
          environment.FileCursor,
          bytesFromFile,
          dataAsBytes
        );
      }
      catch(const std::exception &) {
        environment.Error = std::current_exception();
        return 0; // error state, don't even try for a partial success
      }
    }

    // The virtual file read succeeded. Now we can update our state and shift
    // buffered bytes if we did take any.
    environment.FileCursor += bytesFromFile;
    if(bytesFromBuffer >= 1) {
      environment.BufferedBytes.erase(
        environment.BufferedBytes.begin(),
        environment.BufferedBytes.begin() + byteCount
      );
      return bytesFromBuffer + bytesFromFile;
    } else {
      return bytesFromFile;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the specified number of bytes into the file</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer containing the data to be written</param>
  /// <param name="byteCount">Number of bytes that should be written</param>
  /// <returns>The number of bytes actually written</returns>
  std::int32_t wavPackWriteBytes(void *id, void *data, std::int32_t byteCount) {
    Nuclex::Audio::Storage::WavPack::WriteEnvironment &writeEnvironment = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::WriteEnvironment *
    >(id);
    assert(!writeEnvironment.IsReadOnly && u8"File write is performed on write environment");

    try {
      writeEnvironment.File->WriteAt(
        writeEnvironment.FileCursor,
        byteCount,
        reinterpret_cast<std::uint8_t *>(data)
      );
    }
    catch(const std::exception &) {
      writeEnvironment.Error = std::current_exception();
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
    Nuclex::Audio::Storage::WavPack::SharedEnvironment &environment = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::SharedEnvironment *
    >(id);

    return static_cast<std::int64_t>(environment.FileCursor);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to the specified absolute position</summary>
  /// <typeparam name="TReadOrWriteEnvironment">
  ///   Type of environment the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="position">Absolute position the file cursor should be placed at</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  template<typename TReadOrWriteEnvironment>
  int wavPackSeekAbsolute(void *id, std::int64_t position) {
    TReadOrWriteEnvironment &environment = *reinterpret_cast<
      TReadOrWriteEnvironment *
    >(id);

    // fseek() on POSIX does in fact allow the file cursor to be placed beyond the end of
    // a file. We could emulate this behavior (by simply inserting zero bytes here or doing
    // some acrobatics for read-only streams), but let's, for now, go by the unproven
    // assumption that libwavpack will *not* place the file cursor beyond the end of the file.
    std::uint64_t fileLength = environment.File->GetSize();
    if(fileLength < position) {
      assert((fileLength < position) && u8"Seek keeps file cursor within file boundaries");
      return -1;
    }

    // As documented in the ungetc() function from libc, seeking should discard
    // the effects of ungetc()
    environment.Buffer.clear();

    environment.FileCursor = static_cast<std::uint64_t>(position);
    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor by specified offset</summary>
  /// <typeparam name="TReadOrWriteEnvironment">
  ///   Type of environment the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="delta">Offset by which the file cursor should be placed at</param>
  /// <param name="anchor">Anchor relative to which the file cursor will be placed</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  template<typename TReadOrWriteEnvironment>
  int wavPackSeekRelative(void *id, std::int64_t delta, int anchor) {
    TReadOrWriteEnvironment &environment = *reinterpret_cast<
      TReadOrWriteEnvironment *
    >(id);

    // Should invalid seeks become exceptions to the outside caller?
    //readEnvironment.Error = std::make_exception_ptr(
    //  std::runtime_error(
    //    u8"Relative seek offset would place the file cursor before the start of the file"
    //  )
    //);

    // fseek() on POSIX does in fact allow the file cursor to be placed beyond the end of
    // a file. We could emulate this behavior (by simply inserting zero bytes here or doing
    // some acrobatics for read-only streams), but let's, for now, go by the unproven
    // assumption that libwavpack will *not* place the file cursor beyond the end of the file.
    std::uint64_t newPosition = environment.FileCursor;
    {
      std::uint64_t fileLength = environment.File->GetSize();
      switch(anchor) {
        case SEEK_SET: {
          newPosition = static_cast<std::uint64_t>(delta);
          break;
        }
        case SEEK_CUR: {
          if(delta < 0) {
            if(environment.FileCursor < -delta) {
              std::int64_t fileCursor = environment.FileCursor;
              assert((fileCursor >= -delta) && u8"Seek from end stops at file start");
              return -1;
            }
          }

          newPosition = environment.FileCursor + delta;
          break;
        }
        case SEEK_END: {
          if(delta < 0) {
            if(fileLength < -delta) {
              assert((fileLength >= -delta) && u8"Seek from end stops at file start");
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
    environment.Buffer.clear();

    environment.FileCursor = newPosition;
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
    Nuclex::Audio::Storage::WavPack::SharedEnvironment &environment = *reinterpret_cast<
      Nuclex::Audio::Storage::WavPack::SharedEnvironment *
    >(id);

    if(c >= 256) {
      assert((c < 256) && u8"ungetc() isn't used to buffer an EOF or other value");
      return -1;
    }

    environment.BufferedBytes.push_back(static_cast<std::uint8_t>(c));
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simulates a single byte as if it was in the input stream</summary>
  /// <typeparam name="TReadOrWriteEnvironment">
  ///   Type of environment the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>The length of the input stream in bytes</returns>
  template<typename TReadOrWriteEnvironment>
  std::int64_t wavPackGetLength(void *id) {
    TReadOrWriteEnvironment &environment = *reinterpret_cast<
      TReadOrWriteEnvironment *
    >(id);

    return static_cast<std::int64_t>(environment.File->GetSize());
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the input stream supports seeking</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero is the file is not seekable, any other value if it is</returns>
  int wavPackIsSeekable(void *id) {
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
    assert(u8"TruncateToFileCursor() is never called");
    return -1;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Flushes any unwritten data in internal buffers and closes the file</summary>
  /// <typeparam name="TReadOrWriteEnvironment">
  ///   Type of environment the read is done on (because we don't want to const_cast here)
  /// </typeparam>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero on success, EOF in case of an error</returns>
  template<typename TReadOrWriteEnvironment>
  int wavPackClose(void *id) {
    TReadOrWriteEnvironment *environment = reinterpret_cast<TReadOrWriteEnvironment *>(id);
    delete environment; // This is under the derived type and will release the shared_ptr
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::DoesFileExtensionSayWv(const std::string &extension) {
    bool extensionSaysWv;

    // Microsoft Waveform audio files can have the extension .wav or (rarely) .wave.
    // We'll consider both here.
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 2) { // extension without dot possible
        extensionSaysWv = (
          ((extension[0] == 'w') || (extension[0] == 'W')) &&
          ((extension[1] == 'v') || (extension[1] == 'V'))
        );
      } else if(extensionLength == 3) { // extension with dot or long name possible
        extensionSaysWv = (
          (extension[0] == '.') &&
          ((extension[1] == 'w') || (extension[1] == 'W')) &&
          ((extension[2] == 'v') || (extension[2] == 'V'))
        );
      } else {
        extensionSaysWv = false;
      }
    }

    return extensionSaysWv;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::CheckIfWavPackHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleWavPackSize) {
      return false; // File is too small to be a .wav file
    }

    std::uint8_t fileHeader[16];
    source.ReadAt(0, 16, fileHeader);

    // This checks all headers and magic numbers that are mandatory and makes sure
    // that there is at least one sub-chunk with a valid length - it will probably
    // always be the "fmt" chunk, but we don't want to be the only library that
    // has trouble with a non-standard Waveformat audio file everyone else can load.
    return (
      (fileHeader[0] == 0x77) && //  1 wvpk (file type id)
      (fileHeader[1] == 0x76) && //  2
      (fileHeader[2] == 0x70) && //  3
      (fileHeader[3] == 0x6b) && //  4
      (                          //  - uint32 with block size. *Should* be below 1 MiB.
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 4) < 0x01000000) // 16 MiB
      ) &&
      (                          //  - uint32 for version needed to decode
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8) >= 0x400) &&
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8) < 0x999)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
