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

#include "Nuclex/Audio/Storage/AudioLoader.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      AudioLoader loader;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, GracefullyFailsToLoadGarbageData) {
    AudioLoader loader;

    // This should not lead to an exception because it is an unrecognized file type,
    // rather than a damaged but identifiable file.
    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(u8"Resources/one-megabyte-of-random-bytes.bin")
    );
    EXPECT_FALSE(info.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanLoadWavPackFile) {
    AudioLoader loader;

    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(u8"Resources/5s-silent-5dot1-float.wv")
    );
    EXPECT_TRUE(info.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
