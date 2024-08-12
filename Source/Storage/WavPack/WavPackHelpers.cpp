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

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to specified number of bytes from the file</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer that will receive the data read</param>
  /// <param name="byteCount">Number of bytes that should be read at most</param>
  /// <returns>The number of bytes actually read</returns>
  std::int32_t wavPackReadBytes(void *id, void *data, std::int32_t byteCount) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes the specified number of bytes into the file</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="data">Pointer to a buffer containing the data to be written</param>
  /// <param name="byteCount">Number of bytes that should be written</param>
  /// <returns>The number of bytes actually written</returns>
  std::int32_t WriteBytes(void *id, void *data, std::int32_t byteCount) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the current absolute position of the file cursor</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>The current absolute position of the file cursor in the file</returns>
  std::int64_t GetCurrentPosition(void *id) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to the specified absolute position</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="position">Absolute position the file cursor should be placed at</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  int SeekAbsolute(void *id, std::int64_t position) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor by specified offset</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <param name="delta">Offset by which the file cursor should be placed at</param>
  /// <param name="anchor">Anchor relative to which the file cursor will be placed</param>
  /// <returns>The new absolute position of the file cursor in the file</returns>
  int SeekRelative(void *id, std::int64_t delta, int anchor) {
    throw std::runtime_error(u8"Not implemented yet");
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
  int BufferByte(void *id, int c) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simulates a single byte as if it was in the input stream</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>The length of the input stream in bytes</returns>
  std::int64_t GetLength(void *id) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the input stream supports seeking</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero is the file is not seekable, any other value if it is</returns>
  int IsSeekable(void *id) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Truncates the length fo the input file to the current file cursor</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero on success, -1 in case of an error</returns>
  int TruncateToFileCursor(void *id) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Flushes any unwritten data in internal buffers and closes the file</summary>
  /// <param name="id">User-defined pointer that holds the stream reader adapter</param>
  /// <returns>Zero on success, EOF in case of an error</returns>
  int Close(void *id) {
    throw std::runtime_error(u8"Not implemented yet");
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
