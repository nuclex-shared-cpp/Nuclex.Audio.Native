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

#include "LinuxFileApi.h"

#if defined(NUCLEX_AUDIO_LINUX)

#include "PosixApi.h" // Linux uses Posix error handling
#include "PosixFileApi.h" // for ThrowExceptionForFileAccessError()

#include <linux/limits.h> // for PATH_MAX
#include <fcntl.h> // ::open() and flags
#include <unistd.h> // ::read(), ::write(), ::close(), etc.

#include <cerrno> // To access ::errno directly
#include <vector> // std::vector

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileApi::OpenFileForReading(const std::string &path) {
    int fileDescriptor = ::open(path.c_str(), O_RDONLY | O_LARGEFILE);
    if(unlikely(fileDescriptor < 0)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      Platform::PosixFileApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  int LinuxFileApi::OpenFileForWriting(const std::string &path) {
    int fileDescriptor = ::open(
      path.c_str(),
      O_RDWR | O_CREAT | O_LARGEFILE,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
    );
    if(unlikely(fileDescriptor < 0)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      Platform::PosixFileApi::ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return fileDescriptor;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Seek(int fileDescriptor, ::off_t offset, int anchor) {
    ::off_t absolutePosition = ::lseek(fileDescriptor, offset, anchor);
    if(absolutePosition == -1) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not seek within file", errorNumber
      );
    }

    return static_cast<std::size_t>(absolutePosition);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Read(
    int fileDescriptor, std::byte *buffer, std::size_t count
  ) {
    ssize_t result = ::read(fileDescriptor, buffer, count);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not read data from file via positional read", errorNumber
      );
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::PositionalRead(
    int fileDescriptor, std::byte *buffer, std::size_t count, std::uint64_t offset
  ) {
    ssize_t result = ::pread(fileDescriptor, buffer, count, offset);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not read data from file", errorNumber
      );
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::Write(
    int fileDescriptor, const std::byte *buffer, std::size_t count
  ) {
    ssize_t result = ::write(fileDescriptor, buffer, count);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not write data to file", errorNumber
      );
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t LinuxFileApi::PositionalWrite(
    int fileDescriptor, const std::byte *buffer, std::size_t count, std::uint64_t offset
  ) {
    ssize_t result = ::pwrite(fileDescriptor, buffer, count, offset);
    if(unlikely(result == static_cast<ssize_t>(-1))) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not write data to file via positional write", errorNumber
      );
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t LinuxFileApi::StatFileSize(int fileDescriptor) {
    struct stat fileStatus;

    int failed = ::fstat(fileDescriptor, &fileStatus);
    if(failed) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not query file status", errorNumber
      );
    }

    return static_cast<std::uint64_t>(fileStatus.st_size);
  }

  // ------------------------------------------------------------------------------------------- //

  void LinuxFileApi::Close(int fileDescriptor, bool throwOnError /* = true */) {
    int result = ::close(fileDescriptor);
    if(throwOnError && unlikely(result == -1)) {
      int errorNumber = errno;
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Could not close file", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_LINUX)
