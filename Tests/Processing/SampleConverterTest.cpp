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

#include "Nuclex/Audio/Processing/SampleConverter.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsUnsigned8BitToFloat) {
    std::uint8_t inputSamples[] = { 0, 1, 128, 255 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 8, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -128.0f / 127.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsUnsigned6BitToFloat) {
    std::uint8_t inputSamples[] = { 0, 4, 128, 252 }; // the lesser two bits are padded
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 6, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -32.0f / 31.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsSigned16BitToFloat) {
    std::int16_t inputSamples[] = { -32768, -32767, 0, 32767 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 16, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -32768.0f / 32767.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsSigned12BitToFloat) {
    std::int16_t inputSamples[] = { -32768, -32752, 0, 32752 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 12, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -2048.0f / 2047.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
