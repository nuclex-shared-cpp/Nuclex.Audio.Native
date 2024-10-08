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
#include "Nuclex/Audio/Errors/UnsupportedFormatError.h"

#include "./ResourceDirectoryLocator.h"

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
      loader.TryReadInfo(
        GetResourcesDirectory() + u8"one-hundred-kilobytes-of-random-bytes.bin"
      )
    );
    EXPECT_FALSE(info.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanGetMetadataFromFlac) {
    AudioLoader loader;

    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(
        GetResourcesDirectory() + u8"flac-stereo-int16-v143.flac"
      )
    );

    #if defined(NUCLEX_AUDIO_HAVE_FLAC)
    EXPECT_TRUE(info.has_value());
    #else
    EXPECT_FALSE(info.has_value());
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanOpenDecoderOnFlac) {
    AudioLoader loader;

    std::shared_ptr<AudioTrackDecoder> decoder;
    #if defined(NUCLEX_AUDIO_HAVE_FLAC)
    decoder = loader.OpenDecoder(
      GetResourcesDirectory() + u8"flac-stereo-int16-v143.flac"
    );
    EXPECT_TRUE(static_cast<bool>(decoder));
    #else
    EXPECT_THROW(
      decoder = loader.OpenDecoder(
        GetResourcesDirectory() + u8"flac-stereo-int16-v143.flac"
      ),
      Nuclex::Audio::Errors::UnsupportedFormatError
    );
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanGetMetadataFromOpus) {
    AudioLoader loader;

    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(
        GetResourcesDirectory() + u8"opus-stereo-v152.opus"
      )
    );
    #if defined(NUCLEX_AUDIO_HAVE_OPUS)
    EXPECT_TRUE(info.has_value());
    #else
    EXPECT_FALSE(info.has_value());
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanOpenDecoderOnOpus) {
    AudioLoader loader;

    std::shared_ptr<AudioTrackDecoder> decoder;
    #if defined(NUCLEX_AUDIO_HAVE_OPUS)
    decoder = loader.OpenDecoder(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );
    EXPECT_TRUE(static_cast<bool>(decoder));
    #else
    EXPECT_THROW(
      decoder = loader.OpenDecoder(
        GetResourcesDirectory() + u8"opus-stereo-v152.opus"
      ),
      Nuclex::Audio::Errors::UnsupportedFormatError
    );
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanGetMetadataFromWaveform) {
    AudioLoader loader;

    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(
        GetResourcesDirectory() + u8"waveform-stereo-int16le-pcmwaveformat.wav"
      )
    );
    EXPECT_TRUE(info.has_value());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanGetMetadataFromWavPack) {
    AudioLoader loader;

    std::optional<ContainerInfo> info = (
      loader.TryReadInfo(
        GetResourcesDirectory() + u8"wavpack-stereo-int16-v416.wv"
      )
    );
    #if defined(NUCLEX_AUDIO_HAVE_WAVPACK)
    EXPECT_TRUE(info.has_value());
    #else
    EXPECT_FALSE(info.has_value());
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(AudioLoaderTest, CanOpenDecoderOnWavPack) {
    AudioLoader loader;

    std::shared_ptr<AudioTrackDecoder> decoder;
    #if defined(NUCLEX_AUDIO_HAVE_OPUS)
    decoder = loader.OpenDecoder(
      GetResourcesDirectory() + u8"wavpack-stereo-int16-v416.wv"
    );
    EXPECT_TRUE(static_cast<bool>(decoder));
    #else
    EXPECT_THROW(
      decoder = loader.OpenDecoder(
        GetResourcesDirectory() + u8"wavpack-stereo-int16-v416.wv"
      ),
      Nuclex::Audio::Errors::UnsupportedFormatError
    );
    #endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
