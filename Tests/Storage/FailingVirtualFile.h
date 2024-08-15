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

#ifndef NUCLEX_AUDIO_STORAGE_FAILINGVIRTUALFILE_H
#define NUCLEX_AUDIO_STORAGE_FAILINGVIRTUALFILE_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

#include <stdexcept> // for std::domain_error() used as simulated error

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simulates errors in a virtual file to unit test error cases</summary>
  class FailingVirtualFile : public VirtualFile {

    /// <summary>Initializes a new failing virtual file wrapper</summary>
    public: FailingVirtualFile(const std::shared_ptr<const VirtualFile> &file);
    /// <summary>Initializes a new failing virtual file wrapper</summary>
    public: FailingVirtualFile(const std::shared_ptr<VirtualFile> &file);

    /// <summary>Frees all memory used by the instance</summary>
    public: virtual ~FailingVirtualFile() override = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: std::uint64_t GetSize() const override;

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
    ) const override;

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) override;

    /// <summary>The virtual file to which calls will be forwarded</summary>
    private: std::shared_ptr<const VirtualFile> wrappedReadOnlyFile;
    /// <summary>The virtual file to which calls will be forwarded</summary>
    private: std::shared_ptr<VirtualFile> wrappedWritableFile;

  };

  // ------------------------------------------------------------------------------------------- //

  inline FailingVirtualFile::FailingVirtualFile(const std::shared_ptr<const VirtualFile> &file) :
    wrappedReadOnlyFile(file),
    wrappedWritableFile() {}

  // ------------------------------------------------------------------------------------------- //

  inline FailingVirtualFile::FailingVirtualFile(const std::shared_ptr<VirtualFile> &file) :
    wrappedReadOnlyFile(file),
    wrappedWritableFile(file) {}

  // ------------------------------------------------------------------------------------------- //

  inline std::uint64_t FailingVirtualFile::GetSize() const {
    return this->wrappedReadOnlyFile->GetSize();
  }

  // ------------------------------------------------------------------------------------------- //
  
  inline void FailingVirtualFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
  ) const {
    if((start > 32) || (start + byteCount > 32)) {
      throw std::domain_error(u8"Simulated error from FailingVirtualFile");
    } else {
      return this->wrappedReadOnlyFile->ReadAt(start, byteCount, buffer);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline void FailingVirtualFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
  ) {
    if((start > 32) || (start + byteCount > 32)) {
      throw std::domain_error(u8"Simulated error from FailingVirtualFile");
    } else {
      return this->wrappedWritableFile->WriteAt(start, byteCount, buffer);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_FAILINGVIRTUALFILE_H
