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

#include "PosixFileApi.h"

#if !defined(NUCLEX_AUDIO_WINDOWS)

#include "PosixApi.h"
#include <Nuclex/Support/Errors/FileAccessError.h> // for FileAccessError

#include <cstdio> // for fopen() and fclose()
#include <cerrno> // To access ::errno directly
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForReading(const std::string &path) {
    static const char *fileMode = "rb";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForWriting(const std::string &path) {
    static const char *fileMode = "w+b";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Read(FILE *file, std::uint8_t *buffer, std::size_t count) {
    size_t readByteCount = ::fread(buffer, 1, count, file);
    if(unlikely(readByteCount == 0)) {
      int errorNumber = errno;

      int result = ::feof(file);
      if(result != 0) {
        return 0; // Read was successful, but end of file has been reached
      }

      ThrowExceptionForFileAccessError(
        u8"Could not read data from file", errorNumber
      );
    }

    return static_cast<std::size_t>(readByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Write(FILE *file, const std::uint8_t *buffer, std::size_t count) {
    size_t writtenByteCount = ::fwrite(buffer, 1, count, file);
    if(unlikely(writtenByteCount == 0)) {
      int errorNumber = errno;

      int result = ::ferror(file);
      if(result == 0) {
        return 0; // Write was successful but no bytes could be written ?_?
      }

      ThrowExceptionForFileAccessError(
        u8"Could not write data to file", errorNumber
      );
    }

    return writtenByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Close(FILE *file, bool throwOnError /* = true */) {
    int result = ::fclose(file);
    if(throwOnError && unlikely(result != 0)) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not close file", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Seek(FILE *file, std::ptrdiff_t offset, int anchor) {
    int result = ::fseek(file, static_cast<long>(offset), anchor);
    if(result == -1) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not move file cursor via fseek()", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t PosixFileApi::Tell(FILE *file) {
    long fileCursorPosition = ::ftell(file);
    if(fileCursorPosition == -1) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not query current file cursor position", errorNumber
      );
    }

    return static_cast<std::uint64_t>(fileCursorPosition);
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::ThrowExceptionForFileAccessError(
    const std::string &errorMessage, int errorNumber
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(PosixApi::GetErrorMessage(errorNumber));

    throw Nuclex::Support::Errors::FileAccessError(
      std::error_code(errorNumber, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // !defined(NUCLEX_AUDIO_WINDOWS)
