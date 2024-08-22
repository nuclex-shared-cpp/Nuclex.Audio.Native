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

#ifndef NUCLEX_AUDIO_STORAGE_ENDIANREADER_H
#define NUCLEX_AUDIO_STORAGE_ENDIANREADER_H

#include "Nuclex/Audio/Config.h"

#include <Nuclex/Support/Endian.h> // for Endian::Flip()

#include <cstdint> // for std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class to read numbers from binary data in little endian format</summary>
  class LittleEndianReader {

    /// <summary>Reads an unsigned 8-bit integer value from memory</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory</returns>
    public: static std::uint8_t ReadUInt8(const std::uint8_t *data);

    /// <summary>Reads an unsigned 16-bit integer value in little endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in little endian format</returns>
    public: static std::uint16_t ReadUInt16(const std::uint8_t *data);

    /// <summary>Reads an unsigned 32-bit integer value in little endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in little endian format</returns>
    public: static std::uint32_t ReadUInt32(const std::uint8_t *data);

    /// <summary>Reads an unsigned 64-bit integer value in little endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in little endian format</returns>
    public: static std::uint64_t ReadUInt64(const std::uint8_t *data);

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class to read numbers from binary data in big endian format</summary>
  class BigEndianReader {

    /// <summary>Reads an unsigned 8-bit integer value from memory</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory</returns>
    public: static std::uint8_t ReadUInt8(const std::uint8_t *data);

    /// <summary>Reads an unsigned 16-bit integer value in big endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in big endian format</returns>
    public: static std::uint16_t ReadUInt16(const std::uint8_t *data);

    /// <summary>Reads an unsigned 32-bit integer value in big endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in big endian format</returns>
    public: static std::uint32_t ReadUInt32(const std::uint8_t *data);

    /// <summary>Reads an unsigned 64-bit integer value in big endian format</summary>
    /// <param name="data">Pointer from which the integer will be read</param>
    /// <returns>The integer read from memory in big endian format</returns>
    public: static std::uint64_t ReadUInt64(const std::uint8_t *data);

  };

  // ------------------------------------------------------------------------------------------- //

  inline std::uint8_t LittleEndianReader::ReadUInt8(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return *reinterpret_cast<const std::uint8_t *>(data);
#else
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint8_t *>(data));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint16_t LittleEndianReader::ReadUInt16(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return *reinterpret_cast<const std::uint16_t *>(data);
#else
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint16_t *>(data));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint32_t LittleEndianReader::ReadUInt32(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return *reinterpret_cast<const std::uint32_t *>(data);
#else
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint32_t *>(data));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint64_t LittleEndianReader::ReadUInt64(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return *reinterpret_cast<const std::uint64_t *>(data);
#else
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint64_t *>(data));
#endif
  }


  // ------------------------------------------------------------------------------------------- //
  inline std::uint8_t BigEndianReader::ReadUInt8(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint8_t *>(data));
#else
    return *reinterpret_cast<const std::uint8_t *>(data);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint16_t BigEndianReader::ReadUInt16(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint16_t *>(data));
#else
    return *reinterpret_cast<const std::uint16_t *>(data);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint32_t BigEndianReader::ReadUInt32(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint32_t *>(data));
#else
    return *reinterpret_cast<const std::uint32_t *>(data);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::uint64_t BigEndianReader::ReadUInt64(const std::uint8_t *data) {
#if defined(NUCLEX_AUDIO_LITTLE_ENDIAN)
    return Nuclex::Support::Endian::Flip(*reinterpret_cast<const std::uint64_t *>(data));
#else
    return *reinterpret_cast<const std::uint64_t *>(data);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_ENDIANREADER_H
