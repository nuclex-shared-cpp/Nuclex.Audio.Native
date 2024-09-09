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

  TEST(SampleConverterTest, ConvertsSigned24BitToFloat) {
    std::int32_t inputSamples[] = { -2147483648, -2147483392, 0, 2147483392 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 24, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -8388608.0f / 8388607.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsSigned32BitToFloat) {
    std::int32_t inputSamples[] = { -2147483648, -2147483647, 0, 2147483647 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::Reconstruct(inputSamples, 32, outputSamples, 4);

    EXPECT_EQ(outputSamples[0], -2147483648.0f / 2147483647.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToUnsigned6Bit) {
    float inputSamples[4] = { -32.0f / 31.0f, -1.0f, 0.0f, 1.0f };
    std::uint8_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 6, 4);

    EXPECT_EQ(outputSamples[0], 0);
    EXPECT_EQ(outputSamples[1], 4);
    EXPECT_EQ(outputSamples[2], 128);
    EXPECT_EQ(outputSamples[3], 252);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToUnsigned8Bit) {
    float inputSamples[4] = { -128.0f / 127.0f, -1.0f, 0.0f, 1.0f };
    std::uint8_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 8, 4);

    EXPECT_EQ(outputSamples[0], 0);
    EXPECT_EQ(outputSamples[1], 1);
    EXPECT_EQ(outputSamples[2], 128);
    EXPECT_EQ(outputSamples[3], 255);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToSigned12Bit) {
    float inputSamples[4] = { -2048.0f / 2047.0f, -1.0f, 0.0f, 1.0f };
    std::int16_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 12, 4);

    EXPECT_EQ(outputSamples[0], -32768);
    EXPECT_EQ(outputSamples[1], -32752);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 32752);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToSigned16Bit) {
    float inputSamples[4] = { -32768.0f / 32767.0f, -1.0f, 0.0f, 1.0f };
    std::int16_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 16, 4);

    EXPECT_EQ(outputSamples[0], -32768);
    EXPECT_EQ(outputSamples[1], -32767);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 32767);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToSigned24Bit) {
    float inputSamples[4] = { -8388608.0f / 8388607.0f, -1.0f, 0.0f, 1.0f };
    std::int32_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 24, 4);

    EXPECT_FLOAT_EQ(outputSamples[0], -2147483648); // float accuracy is too low here
    EXPECT_EQ(outputSamples[1], -2147483392);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 2147483392);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToSigned32Bit) {
    float inputSamples[4] = { -2147483648.0f / 2147483647.0f, -1.0f, 0.0f, 1.0f };
    std::int32_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 32, 4);

    EXPECT_FLOAT_EQ(outputSamples[0], -2147483648); // input value already unrepresentable
    EXPECT_EQ(outputSamples[1], -2147483647);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 2147483647);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsDoubleToSigned24Bit) {
    double inputSamples[4] = { -8388608.0 / 8388607.0, -1.0, 0.0, 1.0 };
    std::int32_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 24, 4);

    EXPECT_EQ(outputSamples[0], -2147483648);
    EXPECT_EQ(outputSamples[1], -2147483392);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 2147483392);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsDoubleToSigned32Bit) {
    double inputSamples[4] = { -2147483648.0 / 2147483647.0, -1.0, 0.0, 1.0 };
    std::int32_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::Quantize(inputSamples, outputSamples, 32, 4);

    EXPECT_EQ(outputSamples[0], -2147483648);
    EXPECT_EQ(outputSamples[1], -2147483647);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 2147483647);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsDoubleToFloat) {
    double inputSamples[4] = { -2.0, -1.0, 0.0, 1.0 };
    float outputSamples[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    SampleConverter::TruncateBits(
      inputSamples, sizeof(double) * 8, outputSamples, sizeof(float) * 8, 4
    );

    EXPECT_EQ(outputSamples[0], -2.0f);
    EXPECT_EQ(outputSamples[1], -1.0f);
    EXPECT_EQ(outputSamples[2], 0.0f);
    EXPECT_EQ(outputSamples[3], 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, Converts16BitTo12BitIntegers) {
    std::int16_t inputSamples[4] = { -32768, -32767, 0, 32767 };
    std::int16_t outputSamples[4] = { 0, 0, 0, 0 };

    SampleConverter::TruncateBits(
      inputSamples, 16, outputSamples, 12, 4
    );

    EXPECT_EQ(outputSamples[0], -32768);
    EXPECT_EQ(outputSamples[1], -32752);
    EXPECT_EQ(outputSamples[2], 0);
    EXPECT_EQ(outputSamples[3], 32752);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SampleConverterTest, ConvertsFloatToDouble) {
    float inputSamples[4] = { -2.0f, -1.0f, 0.0f, 1.0f };
    double outputSamples[4] = { 0.0, 0.0, 0.0, 0.0 };

    SampleConverter::ExtendBits(
      inputSamples, sizeof(float) * 8, outputSamples, sizeof(double) * 8, 4
    );

    EXPECT_EQ(outputSamples[0], -2.0);
    EXPECT_EQ(outputSamples[1], -1.0);
    EXPECT_EQ(outputSamples[2], 0.0);
    EXPECT_EQ(outputSamples[3], 1.0);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
