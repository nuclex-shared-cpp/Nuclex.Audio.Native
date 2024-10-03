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

#include "../../../Source/Storage/Vorbis/VorbisTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "../ByteArrayAsFile.h"
#include "../FailingVirtualFile.h"
#include "../ResourceDirectoryLocator.h"
#include "../TestAudioVerifier.h"
#include "../../Processing/SineWaveDetector.h"
#include "../../ExpectRange.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  TEST(VorbisTrackDecoderTest, ExceptionsFromVirtualFileResurface) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"vorbis-stereo-v142.ogg"
    );
    std::shared_ptr<const VirtualFile> failingFile = std::make_shared<FailingVirtualFile>(
      file
    );

    // If the error is forwarded correctly, the domain_error will resurface from the call.
    // Should a plain runtime_error surface here, then error checking was happening but
    // the libvorbisfile error return took precedence over the VirtualFile exception,
    // which is not what we want because it obscures the root cause of the error.
    EXPECT_THROW(
      VorbisTrackDecoder decoder(failingFile),
      std::domain_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VorbisTrackDecoderTest, ReportsChannelOrderForSevenDotOneFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"vorbis-5dot1-v142.ogg"
    );

    VorbisTrackDecoder decoder(file);

    const std::vector<ChannelPlacement> &order = decoder.GetChannelOrder();
    ASSERT_EQ(order.size(), 6U);
    EXPECT_EQ(order[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(order[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(order[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(order[3], ChannelPlacement::BackLeft);
    EXPECT_EQ(order[4], ChannelPlacement::BackRight);
    EXPECT_EQ(order[5], ChannelPlacement::LowFrequencyEffects);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VorbisTrackDecoderTest, DecodesFloatingPoint) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"vorbis-stereo-v142.ogg"
    );

    VorbisTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<float> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    TestAudioVerifier::VerifyStereo(samples, channelCount, 48000);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VorbisTrackDecoderTest, DecodesFloatingPointSeparated) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"vorbis-stereo-v142.ogg"
    );

    VorbisTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();

    std::vector<float> leftSamples(frameCount);
    std::vector<float> rightSamples(frameCount);
    float *channels[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(channels, 0, frameCount);

    TestAudioVerifier::VerifyStereo(leftSamples, rightSamples, 48000);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
