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

#include "Nuclex/Audio//Processing/Reconstruction.h"
#include "../ExpectRange.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanNormalizeIntegerToFloat) {
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(0, 32.0f), 0.0f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(8, 32.0f), 0.25f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(16, 32.0f), 0.5f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(24, 32.0f), 0.75f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(32, 32.0f), 1.0f);

    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(0, 32.0f), 0.0f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-8, 32.0f), -0.25f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-16, 32.0f), -0.5f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-24, 32.0f), -0.75f);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-32, 32.0f), -1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanNormalizeIntegerToDouble) {
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(0, 32.0), 0.0);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(8, 32.0), 0.25);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(16, 32.0), 0.5);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(24, 32.0), 0.75);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(32, 32.0), 1.0);

    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(0, 32.0), 0.0);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-8, 32.0), -0.25);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-16, 32.0), -0.5);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-24, 32.0), -0.75);
    EXPECT_EQ(Reconstruction::DivideInt32ToFloat(-32, 32.0), -1.0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanNormalizeFourIntegersToFloats) {
    std::int32_t values[8] = {
      -0, -128, -256, -384,
      128, 256, 384, 512
    };

    float floats[8];

    Reconstruction::DivideInt32ToFloatx4(values, 512.0f, floats);
    Reconstruction::DivideInt32ToFloatx4(values + 4, 512.0f, floats + 4);

    EXPECT_EQ(floats[0], 0.0f);
    EXPECT_EQ(floats[1], -0.25f);
    EXPECT_EQ(floats[2], -0.5f);
    EXPECT_EQ(floats[3], -0.75f);
    EXPECT_EQ(floats[4], 0.25f);
    EXPECT_EQ(floats[5], 0.5f);
    EXPECT_EQ(floats[6], 0.75f);
    EXPECT_EQ(floats[7], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanNormalizeFourIntegersToDoubles) {
    std::int32_t values[8] = {
      -0, -262144, -524288, -786432,
      262144, 524288, 786432, 1048576
    };

    double doubles[8];

    Reconstruction::DivideInt32ToFloatx4(values, 1048576.0, doubles);
    Reconstruction::DivideInt32ToFloatx4(values + 4, 1048576.0, doubles + 4);

    EXPECT_EQ(doubles[0], 0.0);
    EXPECT_EQ(doubles[1], -0.25);
    EXPECT_EQ(doubles[2], -0.5);
    EXPECT_EQ(doubles[3], -0.75);
    EXPECT_EQ(doubles[4], 0.25);
    EXPECT_EQ(doubles[5], 0.5);
    EXPECT_EQ(doubles[6], 0.75);
    EXPECT_EQ(doubles[7], 1.0);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanNormalizeFourIntegersAsDoublesToFloats) {
    std::int32_t values[8] = {
      -0, -262144, -524288, -786432,
      262144, 524288, 786432, 1048576
    };

    float floats[8];

    Reconstruction::DivideInt32ToFloatx4(values, 1048576.0, floats);
    Reconstruction::DivideInt32ToFloatx4(values + 4, 1048576.0, floats + 4);

    EXPECT_EQ(floats[0], 0.0f);
    EXPECT_EQ(floats[1], -0.25f);
    EXPECT_EQ(floats[2], -0.5f);
    EXPECT_EQ(floats[3], -0.75f);
    EXPECT_EQ(floats[4], 0.25f);
    EXPECT_EQ(floats[5], 0.5f);
    EXPECT_EQ(floats[6], 0.75f);
    EXPECT_EQ(floats[7], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanShiftAndNormalizeFourIntegersToFloats) {
    std::int32_t values[8] = {
      -0, -4194304, -8388608, -12582912,
      4194304, 8388608, 12582912, 16777216
    };

    float floats[8];

    Reconstruction::ShiftAndDivideInt32ToFloatx4(values, 8, 65536.0f, floats);
    Reconstruction::ShiftAndDivideInt32ToFloatx4(values + 4, 8, 65536.0f, floats + 4);

    EXPECT_EQ(floats[0], 0.0f);
    EXPECT_EQ(floats[1], -0.25f);
    EXPECT_EQ(floats[2], -0.5f);
    EXPECT_EQ(floats[3], -0.75f);
    EXPECT_EQ(floats[4], 0.25f);
    EXPECT_EQ(floats[5], 0.5f);
    EXPECT_EQ(floats[6], 0.75f);
    EXPECT_EQ(floats[7], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanShiftAndNormalizeFourIntegersAsDoublesToFloats) {
    std::int32_t values[8] = {
      -0, -4194304, -8388608, -12582912,
      4194304, 8388608, 12582912, 16777216
    };

    float floats[8];

    Reconstruction::ShiftAndDivideInt32ToFloatx4(values, 8, 65536.0, floats);
    Reconstruction::ShiftAndDivideInt32ToFloatx4(values + 4, 8, 65536.0, floats + 4);

    EXPECT_EQ(floats[0], 0.0f);
    EXPECT_EQ(floats[1], -0.25f);
    EXPECT_EQ(floats[2], -0.5f);
    EXPECT_EQ(floats[3], -0.75f);
    EXPECT_EQ(floats[4], 0.25f);
    EXPECT_EQ(floats[5], 0.5f);
    EXPECT_EQ(floats[6], 0.75f);
    EXPECT_EQ(floats[7], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReconstructionTests, CanShiftAndNormalizeFourIntegersToDoubles) {
    std::int32_t values[8] = {
      -0, -4194304, -8388608, -12582912,
      4194304, 8388608, 12582912, 16777216
    };

    double doubles[8];

    Reconstruction::ShiftAndDivideInt32ToFloatx4(values, 8, 65536.0, doubles);
    Reconstruction::ShiftAndDivideInt32ToFloatx4(values + 4, 8, 65536.0, doubles + 4);

    EXPECT_EQ(doubles[0], 0.0);
    EXPECT_EQ(doubles[1], -0.25);
    EXPECT_EQ(doubles[2], -0.5);
    EXPECT_EQ(doubles[3], -0.75);
    EXPECT_EQ(doubles[4], 0.25);
    EXPECT_EQ(doubles[5], 0.5);
    EXPECT_EQ(doubles[6], 0.75);
    EXPECT_EQ(doubles[7], 1.0);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
