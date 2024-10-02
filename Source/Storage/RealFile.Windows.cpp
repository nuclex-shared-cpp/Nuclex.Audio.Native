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

#if defined(NUCLEX_AUDIO_WINDOWS)

#include "../Platform/WindowsFileApi.h"

#include <cassert> // for assert()

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  RealFile::RealFile(
    const std::string &path, bool promiseSequentialAccess, bool readOnly
  ) : position(0) {
    if(readOnly) {
      this->fileHandle = Platform::WindowsFileApi::OpenFileForReading(
        path, promiseSequentialAccess
      );
      this->length = Platform::WindowsFileApi::GetFileSize(this->fileHandle);
    } else {
      this->fileHandle = Platform::WindowsFileApi::OpenFileForWriting(
        path, promiseSequentialAccess
      );
      this->length = 0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  RealFile::~RealFile() {
    BOOL result = ::CloseHandle(this->fileHandle);
    NUCLEX_AUDIO_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"File handle is closed successfully");
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::byte *buffer
  ) const {
    if(start != this->position) {
      Platform::WindowsFileApi::Seek(this->fileHandle, start, FILE_BEGIN);
      this->position = start;
    }
    std::size_t remainingByteCount = byteCount;
    for(;;) {
      std::size_t readByteCount = Platform::WindowsFileApi::Read(
        this->fileHandle, buffer, remainingByteCount
      );
      this->position += readByteCount;

      if(likely(readByteCount == remainingByteCount)) {
        return; // All done
      } else if(unlikely(readByteCount == 0)) {
        Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
          u8"Encountered unexpected end of file", ERROR_HANDLE_EOF
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
    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", ERROR_HANDLE_EOF
        );
      }
      Platform::WindowsFileApi::Seek(this->fileHandle, start, FILE_BEGIN);
    }

    std::size_t writtenByteCount = Platform::WindowsFileApi::Write(
      this->fileHandle, buffer, byteCount
    );
    this->position += writtenByteCount;
    if(this->position > this->length) {
      this->length = this->position;
    }
    if(writtenByteCount != byteCount) {
      Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", ERROR_WRITE_FAULT
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // defined(NUCLEX_AUDIO_WINDOWS)
