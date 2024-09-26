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

#include "Nuclex/Audio//Processing/Rounding.h"
#include "../ExpectRange.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanRoundFloatToNearestInteger) {
    EXPECT_EQ(Rounding::NearestInt32(-0.2f), 0);
    EXPECT_EQ(Rounding::NearestInt32(0.2f), 0);

    EXPECT_EQ(Rounding::NearestInt32(-0.6f), -1);
    EXPECT_EQ(Rounding::NearestInt32(0.6f), 1);

    EXPECT_EQ(Rounding::NearestInt32(-10000.6f), -10001);
    EXPECT_EQ(Rounding::NearestInt32(10000.6f), 10001);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanRoundDoubleToNearestInteger) {
    EXPECT_EQ(Rounding::NearestInt32(-0.2), 0);
    EXPECT_EQ(Rounding::NearestInt32(0.2), 0);

    EXPECT_EQ(Rounding::NearestInt32(-0.6), -1);
    EXPECT_EQ(Rounding::NearestInt32(0.6), 1);

    EXPECT_EQ(Rounding::NearestInt32(-10000.6), -10001);
    EXPECT_EQ(Rounding::NearestInt32(10000.6), 10001);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanRoundFourFloatsToNearestIntegers) {
    float values[8] = {
      -12345.25f, -12345.75f, -2.0f, -0.0f,
      12345.25f, 12345.75f, 2.0f, 0.0f,
    };

    std::int32_t integers[8];

    Rounding::NearestInt32x4(values, integers);
    Rounding::NearestInt32x4(values + 4, integers + 4);

    EXPECT_EQ(integers[0], -12345);
    EXPECT_EQ(integers[1], -12346);
    EXPECT_EQ(integers[2], -2);
    EXPECT_EQ(integers[3], 0);
    EXPECT_EQ(integers[4], 12345);
    EXPECT_EQ(integers[5], 12346);
    EXPECT_EQ(integers[6], 2);
    EXPECT_EQ(integers[7], 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanRoundFourDoublesToNearestIntegers) {
    double values[8] = {
      -12345.25, -12345.75, -2.0, -0.0,
      12345.25, 12345.75, 2.0, 0.0,
    };

    std::int32_t integers[8];

    Rounding::NearestInt32x4(values, integers);
    Rounding::NearestInt32x4(values + 4, integers + 4);

    EXPECT_EQ(integers[0], -12345);
    EXPECT_EQ(integers[1], -12346);
    EXPECT_EQ(integers[2], -2);
    EXPECT_EQ(integers[3], 0);
    EXPECT_EQ(integers[4], 12345);
    EXPECT_EQ(integers[5], 12346);
    EXPECT_EQ(integers[6], 2);
    EXPECT_EQ(integers[7], 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanMultiplyAndRoundFourFloatsToNearestIntegers) {
    float values[8] = {
      -12345.13f, -12345.33f, -2.0f, -0.0f,
      12345.13f, 12345.33f, 2.0f, 0.0f,
    };

    std::int32_t integers[8];

    Rounding::MultiplyToNearestInt32x4(values, 2.4f, integers);
    Rounding::MultiplyToNearestInt32x4(values + 4, 2.4f, integers + 4);

    EXPECT_EQ(integers[0], -29628);
    EXPECT_EQ(integers[1], -29629);
    EXPECT_EQ(integers[2], -5);
    EXPECT_EQ(integers[3], 0);
    EXPECT_EQ(integers[4], 29628);
    EXPECT_EQ(integers[5], 29629);
    EXPECT_EQ(integers[6], 5);
    EXPECT_EQ(integers[7], 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanMultiplyAndRoundFourDoublesToNearestIntegers) {
    double values[8] = {
      -12345.13, -12345.33, -2.0, -0.0,
      12345.13, 12345.33, 2.0, 0.0,
    };

    std::int32_t integers[8];

    Rounding::MultiplyToNearestInt32x4(values, 2.4, integers);
    Rounding::MultiplyToNearestInt32x4(values + 4, 2.4, integers + 4);

    EXPECT_EQ(integers[0], -29628);
    EXPECT_EQ(integers[1], -29629);
    EXPECT_EQ(integers[2], -5);
    EXPECT_EQ(integers[3], 0);
    EXPECT_EQ(integers[4], 29628);
    EXPECT_EQ(integers[5], 29629);
    EXPECT_EQ(integers[6], 5);
    EXPECT_EQ(integers[7], 0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RoundingTests, CanMultiplyAndRoundFourFloatsAsDoublesToNearestIntegers) {
    float values[8] = {
      -2123456.2f, -2123456.8f, -2.0f, -0.0f,
      2123456.2f, 2123456.8f, 2.0f, 0.0f,
    };

    std::int32_t integers[8];

    // Due to the large numbers, the result should be slightly off if the multiplication
    // was peformed on single precision floating point numbers.
    //
    // Practically, it doesn't make any difference...
    Rounding::MultiplyToNearestInt32x4(values, 21.3, integers);
    Rounding::MultiplyToNearestInt32x4(values + 4, 21.3, integers + 4);

    EXPECT_RANGE(integers[0], -45229618, -45229616);
    EXPECT_RANGE(integers[1], -45229630, -45229628);
    EXPECT_EQ(integers[2], -43);
    EXPECT_EQ(integers[3], 0);
    EXPECT_RANGE(integers[4], 45229617, 45229619);
    EXPECT_RANGE(integers[5], 45229629, 45229631);
    EXPECT_EQ(integers[6], 43);
    EXPECT_EQ(integers[7], 0);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
