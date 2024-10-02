#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "RealFile.h"

#if defined(NUCLEX_AUDIO_LINUX)

#include "../Platform/LinuxFileApi.h" // for open(), etc.
#include "../Platform/PosixFileApi.h" // for ThrowExceptionForFileAccessError(), etc.

#include <unistd.h> // ::read(), ::write(), ::close(), etc.

#include <cassert> // for assert()

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  RealFile::RealFile(
    const std::string &path, bool promiseSequentialAccess, bool readOnly
  ) : position(0) {
    (void)promiseSequentialAccess; // Not supported here.

    if(readOnly) {
      this->fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(path);
      this->length = Platform::LinuxFileApi::StatFileSize(this->fileDescriptor);
    } else {
      this->fileDescriptor = Platform::LinuxFileApi::OpenFileForWriting(path);
      this->length = 0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  RealFile::~RealFile() {
    int result = ::close(this->fileDescriptor);
    NUCLEX_AUDIO_NDEBUG_UNUSED(result);
    assert((result != -1) && u8"File descriptor is closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::byte *buffer
  ) const {
    if(start == this->position) { // Prefer read() to support stdin etc.
      std::size_t remainingByteCount = byteCount;
      for(;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
          this->fileDescriptor, buffer, remainingByteCount
        );
        this->position += readByteCount;

        if(likely(readByteCount == remainingByteCount)) {
          return;
        } else if(unlikely(readByteCount == 0)) {
          Platform::PosixFileApi::ThrowExceptionForFileAccessError(
            u8"Encountered unexpected end of file", EIO
          );
        }

        buffer += readByteCount;
        remainingByteCount -= readByteCount;
      }
    } else { // If seeking needed anyway, use pread() instead
      std::size_t remainingByteCount = byteCount;
      for(;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::PositionalRead(
          this->fileDescriptor, buffer, byteCount, start
        );
        if(likely(readByteCount == remainingByteCount)) {
          return;
        } else if(unlikely(readByteCount == 0)) {
          Platform::PosixFileApi::ThrowExceptionForFileAccessError(
            u8"Encountered unexpected end of file", EIO
          );
        }

        buffer += readByteCount;
        start += readByteCount;
        remainingByteCount -= readByteCount;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::byte *buffer
  ) {
    std::size_t writtenByteCount;
    if(start == this->position) { // Prefer write() to support stdout etc.
      writtenByteCount = Platform::LinuxFileApi::Write(
        this->fileDescriptor, buffer, byteCount
      );
      this->position += writtenByteCount;
      if(this->position > this->length) {
        this->length = this->position;
      }
    } else { // If seeking needed anyway, use pwrite()
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", EINVAL
        );
      }
      writtenByteCount = Platform::LinuxFileApi::PositionalWrite(
        this->fileDescriptor, buffer, byteCount, start
      );
      if(start + writtenByteCount > this->length) {
        this->length = start + writtenByteCount;
      }
    }
    if(writtenByteCount != byteCount) {
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", EIO
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // defined(NUCLEX_AUDIO_LINUX)
