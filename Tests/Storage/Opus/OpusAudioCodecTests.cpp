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

#include "../../../Source/Storage/Opus/OpusAudioCodec.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "../FailingVirtualFile.h"
#include "../ResourceDirectoryLocator.h"

#include <gtest/gtest.h>

#include <cstdint> // for std::uint8_t

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The *real* length of the test .opus files in the Resources directory</summary>
  /// <remarks>
  ///   <para>
  ///     I created my audio test files with Audacity and checked them via MediaInfo
  ///     (https://github.com/MediaArea/MediaInfo). Both reported that, after encodign to Opus,
  ///     the audio files had become slightly longer. Thus, I truncated the audio data before
  ///     encoding to obtain precise 1 second Opus audio files.
  ///   </para>
  ///   <para>
  ///     As it turns out, Opus needs a few prio and post samples, but these should be skipped
  ///     by the decoding application. Audacity happily decodes those samples (showing garbage
  ///     or a small repetition at the end) and MediaInfo includes these in the duration.
  ///   </para>
  ///   <para>
  ///     libopusfile, being written by the OGG and Opus developers themselves, does it
  ///     it correctly. The end result is that my audio files are actually 0.993 seconds
  ///     long and Nuclex.Audio.Native reports the correct length, whereas Audacity and
  ///     MediaInfo incorrectly state that the audio files are exactly 1.0 seconds long...
  ///   </para>
  /// </remarks>
  const std::chrono::microseconds ActualOpusTestFileLength(993'000);

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusAudioCodecTest, ExceptionsFromVirtualFileResurface) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );
    std::shared_ptr<const VirtualFile> failingFile = std::make_shared<FailingVirtualFile>(
      file
    );

    // If the error is forwarded correctly, the domain_error will resurface from the call.
    // Should a plain runtime_error surface here, then error checking was happening but
    // the libopusfile error return took precedence over the VirtualFile exception, which is
    // not what we want because it obscures the root cause of the error.
    OpusAudioCodec codec;
    EXPECT_THROW(
      std::optional<ContainerInfo> info = codec.TryReadInfo(failingFile),
      std::domain_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusAudioCodecTest, CanReadInfoFromStereoFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );

    OpusAudioCodec codec;
    std::optional<ContainerInfo> info = codec.TryReadInfo(file);

    ASSERT_TRUE(info.has_value());

    EXPECT_EQ(info.value().Tracks.at(0).ChannelCount, 2U);
    EXPECT_EQ(
      info.value().Tracks.at(0).ChannelPlacements,
      ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight
    );
    EXPECT_TRUE(info.value().Tracks.at(0).Duration == ActualOpusTestFileLength);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusAudioCodecTest, CanReadInfoFromSurroundFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-5dot1-v152.opus"
    );

    OpusAudioCodec codec;
    std::optional<ContainerInfo> info = codec.TryReadInfo(file);

    ASSERT_TRUE(info.has_value());

    EXPECT_EQ(info.value().Tracks.at(0).ChannelCount, 6U);
    EXPECT_EQ(
      info.value().Tracks.at(0).ChannelPlacements,
      (
        ChannelPlacement::FrontCenter |
        ChannelPlacement::FrontLeft |
        ChannelPlacement::FrontRight |
        ChannelPlacement::BackLeft |
        ChannelPlacement::BackRight |
        ChannelPlacement::LowFrequencyEffects
      )
    );

    EXPECT_TRUE(info.value().Tracks.at(0).Duration == ActualOpusTestFileLength);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
