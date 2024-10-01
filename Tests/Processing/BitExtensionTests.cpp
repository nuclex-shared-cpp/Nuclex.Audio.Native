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

#include "Nuclex/Audio//Processing/BitExtension.h"
#include "../ExpectRange.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPattern) {
    std::int32_t input = 0x12300000;
    std::int32_t repeated = BitExtension::RepeatSigned(input, 12, 0x000FFF00);

    EXPECT_EQ(repeated, 0x12312300);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPatternForNegativeIntegers) {
    std::int32_t input = std::int32_t(0x84200000);
    std::int32_t repeated = BitExtension::RepeatSigned(input, 12, 0x000FFF00);

    EXPECT_EQ(repeated, std::int32_t(0x84284200));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPattern) {
    std::int32_t input = 0x12300000;
    std::int32_t repeated = BitExtension::TripleSigned(input, 12, 0x000FFF00);

    EXPECT_EQ(repeated, 0x12312312);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPatternForNegativeIntegers) {
    std::int32_t input = std::int32_t(0x84200000);
    std::int32_t repeated = BitExtension::TripleSigned(input, 12, 0x000FFF00);

    EXPECT_EQ(repeated, std::int32_t(0x84284284));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPatternOfFourIntegers) {
    std::int32_t input[4] = { 0x12300000, 0x23400000, 0x34500000, 0x45600000 };
    std::int32_t repeated[4];

    BitExtension::RepeatSignedx4(input, 12, 0x000FFF00, repeated);

    EXPECT_EQ(repeated[0], 0x12312300);
    EXPECT_EQ(repeated[1], 0x23423400);
    EXPECT_EQ(repeated[2], 0x34534500);
    EXPECT_EQ(repeated[3], 0x45645600);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPatternOfFourNegativeIntegers) {
    std::int32_t input[4] = {
      std::int32_t(0x87600000),
      std::int32_t(0x86400000),
      std::int32_t(0x85200000),
      std::int32_t(0x84000000)
    };
    std::int32_t repeated[4];

    BitExtension::RepeatSignedx4(input, 12, 0x000FFF00, repeated);

    EXPECT_EQ(repeated[0], std::int32_t(0x87687600));
    EXPECT_EQ(repeated[1], std::int32_t(0x86486400));
    EXPECT_EQ(repeated[2], std::int32_t(0x85285200));
    EXPECT_EQ(repeated[3], std::int32_t(0x84084000));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPatternOfFourIntegers) {
    std::int32_t input[4] = { 0x12300000, 0x23400000, 0x34500000, 0x45600000 };
    std::int32_t repeated[4];

    BitExtension::TripleSignedx4(input, 12, 0x000FFF00, repeated);

    EXPECT_EQ(repeated[0], 0x12312312);
    EXPECT_EQ(repeated[1], 0x23423423);
    EXPECT_EQ(repeated[2], 0x34534534);
    EXPECT_EQ(repeated[3], 0x45645645);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPatternOfFourNegativeIntegers) {
    std::int32_t input[4] = {
      std::int32_t(0x87600000),
      std::int32_t(0x86400000),
      std::int32_t(0x85200000),
      std::int32_t(0x84000000)
    };
    std::int32_t repeated[4];

    BitExtension::TripleSignedx4(input, 12, 0x000FFF00, repeated);

    EXPECT_EQ(repeated[0], std::int32_t(0x87687687));
    EXPECT_EQ(repeated[1], std::int32_t(0x86486486));
    EXPECT_EQ(repeated[2], std::int32_t(0x85285285));
    EXPECT_EQ(repeated[3], std::int32_t(0x84084084));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
