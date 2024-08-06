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

#include "../../../Source/Storage/WavPack/WavPackHelpers.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Binary contents of the smallest possible WavPack file</summary>
  std::uint8_t smallestPossibleWavPackFile[118] = {
    0x77, 0x76, 0x70, 0x6B, 0x6E, 0x00, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x05, 0x18, 0x80, 0x14, 0xF7, 0xFF, 0xFF, 0xFF, 0x68, 0x02, 0x77, 0x61,
    0x76, 0x00, 0x21, 0x16, 0x52, 0x49, 0x46, 0x46, 0x28, 0x00, 0x00, 0x00,
    0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00,
    0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x04, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x25, 0x02, 0x08, 0x00, 0x02, 0x01, 0x2A, 0x00, 0x8A, 0x01,
    0x00, 0x00, 0xF3, 0xFF, 0x2F, 0x02, 0xC3, 0xED, 0x9B, 0xC9
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

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackHelpersTest, DetectsWavPackFiles) {
    {
      std::uint8_t dummyData[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5
      };
      const InMemoryFile dummyFile(dummyData, sizeof(dummyData));
      EXPECT_FALSE(Helpers::CheckIfWavPackHeaderPresent(dummyFile));
    }

    {
      const InMemoryFile flacFile(
        smallestPossibleWavPackFile, sizeof(smallestPossibleWavPackFile)
      );
      EXPECT_TRUE(Helpers::CheckIfWavPackHeaderPresent(flacFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack
