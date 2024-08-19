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

#include "../../../Source/Storage/Flac/FlacDetection.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/Storage/VirtualFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Binary contents of the smallest possible FLAC file</summary>
  std::uint8_t smallestPossibleFlacFile[86] = {
    0x66, 0x4C, 0x61, 0x43, 0x00, 0x00, 0x00, 0x22, 0x10, 0x00, 0x10, 0x00,
    0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x0A, 0xC4, 0x40, 0xF0, 0x00, 0x00,
    0x00, 0x00, 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04, 0xE9, 0x80,
    0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E, 0x84, 0x00, 0x00, 0x28, 0x20, 0x00,
    0x00, 0x00, 0x72, 0x65, 0x66, 0x65, 0x72, 0x65, 0x6E, 0x63, 0x65, 0x20,
    0x6C, 0x69, 0x62, 0x46, 0x4C, 0x41, 0x43, 0x20, 0x31, 0x2E, 0x34, 0x2E,
    0x33, 0x20, 0x32, 0x30, 0x32, 0x33, 0x30, 0x36, 0x32, 0x33, 0x00, 0x00,
    0x00, 0x00
  };

  // ------------------------------------------------------------------------------------------- //
    
  /// <summary>Simple virtual file implementation that accesses an in-memory buffer</summary>
  class InMemoryFile : public Nuclex::Audio::Storage::VirtualFile {

    /// <summary>Initializes a new memory buffer based file</summary>
    /// <param name="data">Memory buffer the virtual file will access</param>
    /// <param name="length">Size of the memory buffer in bytes</param>
    public: InMemoryFile(const std::uint8_t *data, std::uint64_t length) :
      data(data),
      length(length) {}

    /// <summary>Frees all memory used by the instance</summary>
    public: ~InMemoryFile() override = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: std::uint64_t GetSize() const override { return this->length; }

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
    ) const override {
      std::copy_n(this->data + start, byteCount, buffer);
    }

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) override {
      (void)start;
      (void)byteCount;
      (void)buffer;
      assert(!u8"Write method of unit test dummy file is never called");
    }

    /// <summary>Memory buffer the virtual file implementation is serving data from</summary>
    private: const std::uint8_t *data;
    /// <summary>Length of the memory buffer in bytes</summary>
    private: std::uint64_t length;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  TEST(FlacDetectionTest, DetectsFlacFiles) {
    {
      std::uint8_t dummyData[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5
      };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      EXPECT_FALSE(Detection::CheckIfFlacHeaderPresent(dummyFile));
    }

    {
      const InMemoryFile flacFile(smallestPossibleFlacFile, sizeof(smallestPossibleFlacFile));
      EXPECT_TRUE(Detection::CheckIfFlacHeaderPresent(flacFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif defined(NUCLEX_AUDIO_HAVE_FLAC)
