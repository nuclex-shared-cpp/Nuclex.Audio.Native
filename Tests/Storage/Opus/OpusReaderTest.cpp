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

#include "../../../Source/Storage/Opus/OpusReader.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "../ByteArrayAsFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {


  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromOneChannel) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 1)
    );

    ASSERT_EQ(placement.size(), 1U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontCenter);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromTwoChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 2)
    );

    ASSERT_EQ(placement.size(), 2U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontRight);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromThreeChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 3)
    );

    ASSERT_EQ(placement.size(), 3U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(placement[2], ChannelPlacement::FrontRight);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromFourChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 4)
    );

    ASSERT_EQ(placement.size(), 4U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontRight);
    EXPECT_EQ(placement[2], ChannelPlacement::BackLeft);
    EXPECT_EQ(placement[3], ChannelPlacement::BackRight);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromFiveChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 5)
    );

    ASSERT_EQ(placement.size(), 5U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(placement[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(placement[3], ChannelPlacement::BackLeft);
    EXPECT_EQ(placement[4], ChannelPlacement::BackRight);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromSixChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 6)
    );

    ASSERT_EQ(placement.size(), 6U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(placement[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(placement[3], ChannelPlacement::BackLeft);
    EXPECT_EQ(placement[4], ChannelPlacement::BackRight);
    EXPECT_EQ(placement[5], ChannelPlacement::LowFrequencyEffects);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromSevenChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 7)
    );

    ASSERT_EQ(placement.size(), 7U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(placement[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(placement[3], ChannelPlacement::SideLeft);
    EXPECT_EQ(placement[4], ChannelPlacement::SideRight);
    EXPECT_EQ(placement[5], ChannelPlacement::BackCenter);
    EXPECT_EQ(placement[6], ChannelPlacement::LowFrequencyEffects);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderCanBeDeducedFromEightChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 8)
    );

    ASSERT_EQ(placement.size(), 8U);
    EXPECT_EQ(placement[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(placement[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(placement[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(placement[3], ChannelPlacement::SideLeft);
    EXPECT_EQ(placement[4], ChannelPlacement::SideRight);
    EXPECT_EQ(placement[5], ChannelPlacement::BackLeft);
    EXPECT_EQ(placement[6], ChannelPlacement::BackRight);
    EXPECT_EQ(placement[7], ChannelPlacement::LowFrequencyEffects);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusReaderTest, ChannelOrderDefaultsAtNineChannels) {
    std::vector<ChannelPlacement> placement = (
      OpusReader::ChannelOrderFromMappingFamilyAndChannelCount(0, 9)
    );

    ASSERT_EQ(placement.size(), 9U);
    for(std::size_t index = 0; index < 9; ++index) {
      EXPECT_EQ(placement[index], ChannelPlacement::Unknown);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
