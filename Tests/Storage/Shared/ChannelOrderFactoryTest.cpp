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

#include "../../../Source/Storage/Shared/ChannelOrderFactory.h"

#include "../ByteArrayAsFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderFactoryTest, ChannelsAreOrderedLikeWaveformatChannelMask) {
    std::vector<ChannelPlacement> channelOrder = (
      ChannelOrderFactory::FromWaveformatExtensibleLayout(
        4,
        (
          ChannelPlacement::FrontLeft |
          ChannelPlacement::FrontCenter |
          ChannelPlacement::BackCenter |
          ChannelPlacement::SideLeft
        )
      )
    );

    ASSERT_EQ(channelOrder.size(), 4U);
    EXPECT_EQ(channelOrder[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(channelOrder[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(channelOrder[2], ChannelPlacement::BackCenter);
    EXPECT_EQ(channelOrder[3], ChannelPlacement::SideLeft);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderFactoryTest, ChannelsMissingInMaskAreAppendedAtEnd) {
    std::vector<ChannelPlacement> channelOrder = (
      ChannelOrderFactory::FromWaveformatExtensibleLayout(
        5,
        (
          ChannelPlacement::FrontLeft |
          ChannelPlacement::FrontRight
        )
      )
    );

    ASSERT_EQ(channelOrder.size(), 5U);
    EXPECT_EQ(channelOrder[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(channelOrder[1], ChannelPlacement::FrontRight);
    EXPECT_EQ(channelOrder[2], ChannelPlacement::Unknown);
    EXPECT_EQ(channelOrder[3], ChannelPlacement::Unknown);
    EXPECT_EQ(channelOrder[4], ChannelPlacement::Unknown);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderFactoryTest, EmptyChannelMaskisValid) {
    std::vector<ChannelPlacement> channelOrder = (
      ChannelOrderFactory::FromWaveformatExtensibleLayout(
        2, static_cast<ChannelPlacement>(0)
      )
    );

    ASSERT_EQ(channelOrder.size(), 2U);
    EXPECT_EQ(channelOrder[0], ChannelPlacement::Unknown);
    EXPECT_EQ(channelOrder[1], ChannelPlacement::Unknown);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared

