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

#include "../../../Source/Storage/Opus/OpusTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "../ByteArrayAsFile.h"
#include "../FailingVirtualFile.h"
#include "../ResourceDirectoryLocator.h"
#include "../../Processing/SineWaveDetector.h"
#include "../../ExpectRange.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, ExceptionsFromVirtualFileResurface) {
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
    EXPECT_THROW(
      OpusTrackDecoder decoder(failingFile),
      std::domain_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, ReportsChannelOrderForSevenDotOneFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-7dot1-v152.opus"
    );

    OpusTrackDecoder decoder(file);

    // This will match the Vorbis channel order rather than the one found in Waveform and
    // WavPack files (which is based on the WaveFormatExtensible channel mask)
    const std::vector<ChannelPlacement> &order = decoder.GetChannelOrder();
    ASSERT_EQ(order.size(), 8U);
    EXPECT_EQ(order[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(order[1], ChannelPlacement::FrontCenter);
    EXPECT_EQ(order[2], ChannelPlacement::FrontRight);
    EXPECT_EQ(order[3], ChannelPlacement::SideLeft);
    EXPECT_EQ(order[4], ChannelPlacement::SideRight);
    EXPECT_EQ(order[5], ChannelPlacement::BackLeft);
    EXPECT_EQ(order[6], ChannelPlacement::BackRight);
    EXPECT_EQ(order[7], ChannelPlacement::LowFrequencyEffects);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, DecodesToInterleavedInt16) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );

    OpusTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<std::int16_t> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, DecodesToSeparatedInt16) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );

    OpusTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<std::int16_t> leftSamples(frameCount);
    std::vector<std::int16_t> rightSamples(frameCount);
    std::int16_t *samples[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(samples, 0, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, DecodesToSeparatedFloat) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );

    OpusTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<float> leftSamples(frameCount);
    std::vector<float> rightSamples(frameCount);
    float *samples[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(samples, 0, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackDecoderTest, DecodesFloatingPoint) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"opus-stereo-v152.opus"
    );

    OpusTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<float> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    // TODO: The sine wave detector is not robust enough to correctly detect
    //   the properties of the decoded Opus audio data. It reports a much higher
    //   frequency, even though the number of zero crossing I count in Audacity
    //   is correct and data lines up with audaicty (after fixing Audacity's
    //   Opus import bug, that is).

    // Left signal should be at 0° phase, 25 Hz and have an amplitude of 1.0
    {
      Processing::SineWaveDetector left;
      left.DetectAmplitude(samples.data(), frameCount * 2, channelCount);
      left.AddSamples(samples.data(), frameCount * 2, channelCount);

      //EXPECT_RANGE(left.GetFrequency(48000), 24.8f, 25.2f);
      EXPECT_RANGE(left.GetAmplitude(), 0.9f, 1.1f);
      //EXPECT_RANGE(left.GetPhase360(), -0.5f, 0.5f);
      //EXPECT_LT(left.GetError(), 0.0001f);
    }

    // Right signal should be at 180° phase, 25 Hz and have an amplitude of 1.0
    {
      Processing::SineWaveDetector right;
      right.DetectAmplitude(samples.data() + 1, frameCount * 2, channelCount);
      right.AddSamples(samples.data() + 1, frameCount * 2, channelCount);

      //EXPECT_RANGE(right.GetFrequency(48000), 24.8f, 25.2f);
      EXPECT_RANGE(right.GetAmplitude(), 0.9f, 1.1f);
      //EXPECT_RANGE(right.GetPhase360(), 179.5f, 180.5f); // or -180.0 .. -179.5...
      //EXPECT_LT(right.GetError(), 0.0001f);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
