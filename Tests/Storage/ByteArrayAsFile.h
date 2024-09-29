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

#ifndef NUCLEX_AUDIO_STORAGE_BYTEARRAYASFILE_H
#define NUCLEX_AUDIO_STORAGE_BYTEARRAYASFILE_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

#include <stdexcept> // for std::domain_error() used as simulated error
#include <cstdint>

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Simple virtual file implementation that serves a byte array as file</summary>
  class ByteArrayAsFile : public Nuclex::Audio::Storage::VirtualFile {

    /// <summary>Initializes a new memory buffer based file</summary>
    /// <param name="data">Memory buffer the virtual file will access</param>
    /// <param name="length">Size of the memory buffer in bytes</param>
    public: ByteArrayAsFile(const std::byte *data, std::uint64_t length) :
      data(data),
      length(length) {}

    /// <summary>Frees all memory used by the instance</summary>
    public: ~ByteArrayAsFile() override = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: std::uint64_t GetSize() const override { return this->length; }

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::byte *buffer
    ) const override;

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::byte *buffer
    ) override;

    /// <summary>Memory buffer the virtual file implementation is serving data from</summary>
    private: const std::byte *data;
    /// <summary>Length of the memory buffer in bytes</summary>
    private: std::uint64_t length;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_BYTEARRAYASFILE_H
