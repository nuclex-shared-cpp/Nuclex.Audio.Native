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

#if !defined(NUCLEX_AUDIO_LINUX) && !defined(NUCLEX_AUDIO_WINDOWS)

#include "../Platform/PosixFileApi.h" // for ThrowExceptionForFileAccessError(), etc.

#include <cassert> // for assert()

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  RealFile::RealFile(
    const std::string &path, bool promiseSequentialAccess, bool readOnly
  ) : position(0) {
    (void)promiseSequentialAccess; // Not supported here.
    if(readOnly) {
      this->file = Platform::PosixFileApi::OpenFileForReading(path);
      Platform::PosixFileApi::Seek(this->file, 0, SEEK_END);
      this->length = Platform::PosixFileApi::Tell(this->file);
      Platform::PosixFileApi::Seek(this->file, 0, SEEK_SET);
    } else {
      this->file = Platform::PosixFileApi::OpenFileForWriting(path);
      this->length = 0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  RealFile::~RealFile() {
    int result = ::fclose(this->file);
    NUCLEX_AUDIO_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"File is closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::byte *buffer
  ) const {
    std::lock_guard<std::mutex> readMutexScope(this->readMutex);

    if(start != this->position) {
      Platform::PosixFileApi::Seek(this->file, start, SEEK_SET);
      this->position = start;
    }

    std::size_t remainingByteCount = byteCount;
    for(;;) {
      std::size_t readByteCount = Platform::PosixFileApi::Read(
        this->file, buffer, remainingByteCount
      );
      this->position += readByteCount;

      if(likely(readByteCount == remainingByteCount)) {
        return; // All done
      } else if(unlikely(readByteCount == 0)) {
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Encountered unexpected end of file", EIO
        );
      }

      remainingByteCount -= readByteCount;
      buffer -= readByteCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::byte *buffer
  ) {
    std::lock_guard<std::mutex> readMutexScope(this->readMutex);

    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", EIO
        );
      }
      Platform::PosixFileApi::Seek(this->file, start, SEEK_SET);
    }

    std::size_t writtenByteCount = Platform::PosixFileApi::Write(
      this->file, buffer, byteCount
    );
    this->position += writtenByteCount;
    if(this->position > this->length) {
      this->length = this->position;
    }
    if(writtenByteCount != byteCount) {
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", EIO
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // !defined(NUCLEX_AUDIO_LINUX) && !defined(NUCLEX_AUDIO_WINDOWS)
