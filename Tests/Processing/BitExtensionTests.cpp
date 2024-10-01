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
    std::int32_t repeated = BitExtension::RepeatSigned(input, 11, 0x000FFE00);

    EXPECT_EQ(repeated, 0x12324600);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPatternForNegativeIntegers) {
    std::int32_t input = std::int32_t(0x84200000);
    std::int32_t repeated = BitExtension::RepeatSigned(input, 11, 0x000FFE00);

    EXPECT_EQ(repeated, std::int32_t(0x84208400));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPattern) {
    std::int32_t input = 0x12300000;
    std::int32_t repeated = BitExtension::TripleSigned(input, 11, 0x000FFE00);

    EXPECT_EQ(repeated, 0x12324648);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPatternForNegativeIntegers) {
    std::int32_t input = std::int32_t(0x84200000);
    std::int32_t repeated = BitExtension::TripleSigned(input, 11, 0x000FFE00);

    EXPECT_EQ(repeated, std::int32_t(0x84208410));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanRepeatBitPatternOfFourIntegers) {
    std::int32_t input[4] = { 0x12300000, 0x23400000, 0x34500000, 0x45600000 };
    std::int32_t repeated[4];

    BitExtension::RepeatSignedx4(input, 11, 0x000FFE00, repeated);

    EXPECT_EQ(repeated[0], 0x12324600);
    EXPECT_EQ(repeated[1], 0x23446800);
    EXPECT_EQ(repeated[2], 0x34568A00);
    EXPECT_EQ(repeated[3], 0x4568AC00);
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

    BitExtension::RepeatSignedx4(input, 11, 0x000FFE00, repeated);

    EXPECT_EQ(repeated[0], std::int32_t(0x8760EC00));
    EXPECT_EQ(repeated[1], std::int32_t(0x8640C800));
    EXPECT_EQ(repeated[2], std::int32_t(0x8520A400));
    EXPECT_EQ(repeated[3], std::int32_t(0x84008000));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitExtensionTests, CanTripleBitPatternOfFourIntegers) {
    std::int32_t input[4] = { 0x12300000, 0x23400000, 0x34500000, 0x45600000 };
    std::int32_t repeated[4];

    BitExtension::TripleSignedx4(input, 11, 0x000FFE00, repeated);

    EXPECT_EQ(repeated[0], 0x12324648);
    EXPECT_EQ(repeated[1], 0x2344688D);
    EXPECT_EQ(repeated[2], 0x34568AD1);
    EXPECT_EQ(repeated[3], 0x4568AD15);
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

    BitExtension::TripleSignedx4(input, 11, 0x000FFE00, repeated);

    EXPECT_EQ(repeated[0], std::int32_t(0x8760EC1D));
    EXPECT_EQ(repeated[1], std::int32_t(0x8640C819));
    EXPECT_EQ(repeated[2], std::int32_t(0x8520A414));
    EXPECT_EQ(repeated[3], std::int32_t(0x84008010));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
