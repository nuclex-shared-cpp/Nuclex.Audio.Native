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

#include "Nuclex/Audio/Storage/AudioSaver.h"
#include "Nuclex/Audio/Errors/UnsupportedFormatError.h"

#include "./ResourceDirectoryLocator.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioSaverTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      AudioSaver saver;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioSaverTest, CanListAvailableCodecs) {
    AudioSaver saver;

    std::vector<std::string> codecNames = saver.GetAvailableCodecs();
    EXPECT_GE(codecNames.size(), 1U);
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_AUDIO_HAVE_OPUS)
  TEST(AudioSaverTest, CanObtainOpusEncoderBuilder) {
    AudioSaver saver;

    std::shared_ptr<AudioTrackEncoderBuilder> builder = saver.ProvideBuilder(u8"Opus");
    EXPECT_TRUE(static_cast<bool>(builder));

    std::vector<std::string> codecNames = saver.GetAvailableCodecs();
    EXPECT_GE(codecNames.size(), 1U);
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
