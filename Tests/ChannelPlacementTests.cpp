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

#include "Nuclex/Audio/ChannelPlacement.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelPlacementTest, PlacementCanBeConvertedToString) {
    std::string frontLeft = StringFromChannelPlacement(ChannelPlacement::FrontLeft);

    EXPECT_TRUE(frontLeft.find(u8"front") != std::string::npos);
    EXPECT_TRUE(frontLeft.find(u8"left") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelPlacementTest, PlacementMaskCanBeConvrtedToString) {
    std::string fiveDotOne = StringFromChannelPlacement(
      ChannelPlacement::FrontLeft |
      ChannelPlacement::FrontRight |
      ChannelPlacement::FrontCenter |
      ChannelPlacement::LowFrequencyEffects |
      ChannelPlacement::BackLeft |
      ChannelPlacement::BackRight
    );

    EXPECT_TRUE(fiveDotOne.find(u8"front left") != std::string::npos);
    EXPECT_TRUE(fiveDotOne.find(u8"front right") != std::string::npos);
    EXPECT_TRUE(fiveDotOne.find(u8"front center") != std::string::npos);
    EXPECT_TRUE(fiveDotOne.find(u8"low frequency") != std::string::npos);
    EXPECT_TRUE(fiveDotOne.find(u8"back left") != std::string::npos);
    EXPECT_TRUE(fiveDotOne.find(u8"back right") != std::string::npos);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelPlacementTest, PlacementCanBeParsedFromString) {
    std::string frontLeft(u8"front left", 10);

    ChannelPlacement placement = ChannelPlacementFromString(frontLeft);

    EXPECT_EQ(placement, ChannelPlacement::FrontLeft);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelPlacementTest, CombinedPlacementFlagsCanBeParsedFromString) {
    std::string frontLeft(
      u8"front left, front center right, back center, low frequency effects", 66
    );

    ChannelPlacement placement = ChannelPlacementFromString(frontLeft);

    ChannelPlacement expected = (
      ChannelPlacement::FrontLeft |
      ChannelPlacement::FrontCenterRight |
      ChannelPlacement::BackCenter |
      ChannelPlacement::LowFrequencyEffects
    );
    EXPECT_EQ(placement, expected);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio
