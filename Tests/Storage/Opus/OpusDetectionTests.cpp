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

#include "../../../Source/Storage/Opus/OpusDetection.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "../ByteArrayAsFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Binary contents of a pretty small OPUS file</summary>
  std::uint8_t prettySmallOpusFile[876] = {
    0x4F, 0x67, 0x67, 0x53, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xC4, 0x94, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x92, 0xA3,
    0x5E, 0xCA, 0x01, 0x13, 0x4F, 0x70, 0x75, 0x73, 0x48, 0x65, 0x61, 0x64,
    0x01, 0x01, 0x38, 0x01, 0x44, 0xAC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F,
    0x67, 0x67, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xE0, 0xC4, 0x94, 0x6E, 0x01, 0x00, 0x00, 0x00, 0x9A, 0x31, 0x87,
    0x12, 0x03, 0xFF, 0xFF, 0xFE, 0x4F, 0x70, 0x75, 0x73, 0x54, 0x61, 0x67,
    0x73, 0x1F, 0x00, 0x00, 0x00, 0x6C, 0x69, 0x62, 0x6F, 0x70, 0x75, 0x73,
    0x20, 0x31, 0x2E, 0x35, 0x2E, 0x32, 0x2C, 0x20, 0x6C, 0x69, 0x62, 0x6F,
    0x70, 0x75, 0x73, 0x65, 0x6E, 0x63, 0x20, 0x30, 0x2E, 0x32, 0x2E, 0x31,
    0x02, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x45, 0x4E, 0x43, 0x4F,
    0x44, 0x45, 0x52, 0x3D, 0x6F, 0x70, 0x75, 0x73, 0x65, 0x6E, 0x63, 0x20,
    0x66, 0x72, 0x6F, 0x6D, 0x20, 0x6F, 0x70, 0x75, 0x73, 0x2D, 0x74, 0x6F,
    0x6F, 0x6C, 0x73, 0x20, 0x30, 0x2E, 0x32, 0x21, 0x00, 0x00, 0x00, 0x45,
    0x4E, 0x43, 0x4F, 0x44, 0x45, 0x52, 0x5F, 0x4F, 0x50, 0x54, 0x49, 0x4F,
    0x4E, 0x53, 0x3D, 0x2D, 0x2D, 0x76, 0x62, 0x72, 0x20, 0x2D, 0x2D, 0x62,
    0x69, 0x74, 0x72, 0x61, 0x74, 0x65, 0x20, 0x31, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x4F, 0x67, 0x67, 0x53, 0x00, 0x04, 0x3B, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xE0, 0xC4, 0x94, 0x6E, 0x02, 0x00, 0x00, 0x00, 0x97,
    0x49, 0xE6, 0xF7, 0x01, 0x07, 0x08, 0x0B, 0xF1, 0xF9, 0x16, 0x85, 0xE0
  };

  // ------------------------------------------------------------------------------------------- //
    
} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusDetectionTest, DetectsOpusFilesWithoutLibOpusFile) {
    {
      std::uint8_t dummyData[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5
      };
      const ByteArrayAsFile dummyFile(
        reinterpret_cast<const std::byte *>(dummyData), sizeof(dummyData)
      );
      EXPECT_FALSE(Detection::CheckIfOpusHeaderPresentLite(dummyFile));
    }

    {
      const ByteArrayAsFile opusFile(
        reinterpret_cast<const std::byte *>(prettySmallOpusFile), sizeof(prettySmallOpusFile)
      );
      EXPECT_TRUE(Detection::CheckIfOpusHeaderPresentLite(opusFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusDetectionTest, DetectsOpusFiles) {
    {
      std::uint8_t dummyData[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5
      };
      const ByteArrayAsFile dummyFile(
        reinterpret_cast<const std::byte *>(dummyData), sizeof(dummyData)
      );
      EXPECT_FALSE(Detection::CheckIfOpusHeaderPresent(dummyFile));
    }

    {
      const ByteArrayAsFile opusFile(
        reinterpret_cast<const std::byte *>(prettySmallOpusFile),
        sizeof(prettySmallOpusFile)
      );
      EXPECT_TRUE(Detection::CheckIfOpusHeaderPresent(opusFile));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
