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

#include "../../../Source/Storage/Shared/ChannelOrderTransformer.h"

#include "../ByteArrayAsFile.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if discrepancies in the channel order are detected</summary>
  /// <param name="order">Channels in the proper order</param>
  void testWaveformatExtensibleChannelOrder(
    const std::vector<Nuclex::Audio::ChannelPlacement> &order
  ) {
    using Nuclex::Audio::Storage::Shared::ChannelOrderTransformer;

    // The layout should have been passed in the proper order, so testing the order
    // we've been provided with should return a match
    EXPECT_TRUE(ChannelOrderTransformer::IsWaveformatExtensibleLayout(order));

    // Now go through the list any flip one channel after another to see if
    // the checking method will detect the discrepancy
    for(std::size_t index = 0; index < order.size(); ++index) {
      std::vector<Nuclex::Audio::ChannelPlacement> copy(order);
      if(index == 0) {
        copy[0] = Nuclex::Audio::ChannelPlacement::LowFrequencyEffects;
      } else {
        Nuclex::Audio::ChannelPlacement temp = copy[index - 1];
        copy[index - 1] = copy[index];
        copy[index] = temp;
      }

      EXPECT_FALSE(ChannelOrderTransformer::IsWaveformatExtensibleLayout(copy));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if discrepancies in the channel order are detected</summary>
  /// <param name="order">Channels in the proper order</param>
  void testVorbisChannelOrder(
    const std::vector<Nuclex::Audio::ChannelPlacement> &order
  ) {
    using Nuclex::Audio::Storage::Shared::ChannelOrderTransformer;

    // The layout should have been passed in the proper order, so testing the order
    // we've been provided with should return a match
    EXPECT_TRUE(ChannelOrderTransformer::IsVorbisLayout(order));

    // Now go through the list any flip one channel after another to see if
    // the checking method will detect the discrepancy
    for(std::size_t index = 0; index < order.size(); ++index) {
      std::vector<Nuclex::Audio::ChannelPlacement> copy(order);
      if(index == 0) {
        copy[0] = Nuclex::Audio::ChannelPlacement::LowFrequencyEffects;
      } else {
        Nuclex::Audio::ChannelPlacement temp = copy[index - 1];
        copy[index - 1] = copy[index];
        copy[index] = temp;
      }

      EXPECT_FALSE(ChannelOrderTransformer::IsVorbisLayout(copy));
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectWaveformatExtensibleStereoLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight
    };

    testWaveformatExtensibleChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectWaveformatExtensibleFiveDotOneLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::LowFrequencyEffects,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight
    };

    testWaveformatExtensibleChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectWaveformatExtensibleMaximalLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::LowFrequencyEffects,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight,
      ChannelPlacement::FrontCenterLeft,
      ChannelPlacement::FrontCenterRight,
      ChannelPlacement::BackCenter,
      ChannelPlacement::SideLeft,
      ChannelPlacement::SideRight,
      ChannelPlacement::TopCenter,
      ChannelPlacement::TopFrontLeft,
      ChannelPlacement::TopFrontCenter,
      ChannelPlacement::TopFrontRight,
      ChannelPlacement::TopBackLeft,
      ChannelPlacement::TopBackCenter,
      ChannelPlacement::TopBackRight
    };

    testWaveformatExtensibleChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectVorbisStereoLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight
    };

    testVorbisChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectVorbisFiveDotOneLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::FrontRight,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight,
      ChannelPlacement::LowFrequencyEffects
    };

    testVorbisChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelOrderTransformerTests, CanDetectVorbisSevenDotOneLayouts) {
    std::vector<ChannelPlacement> order = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::FrontRight,
      ChannelPlacement::SideLeft,
      ChannelPlacement::SideRight,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight,
      ChannelPlacement::LowFrequencyEffects
    };

    testVorbisChannelOrder(order);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared

