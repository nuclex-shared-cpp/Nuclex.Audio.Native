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

#include "../../../Source/Storage/Waveform/WaveformTrackDecoder.h"

#include "../ByteArrayAsFile.h"
#include "../FailingVirtualFile.h"
#include "../ResourceDirectoryLocator.h"
#include "../TestAudioVerifier.h"
#include "../../Processing/SineWaveDetector.h"
#include "../../ExpectRange.h"

#include "Nuclex/Audio/Processing/SampleConverter.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  TEST(WaveformTrackDecoderTest, ExceptionsFromVirtualFileResurface) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"waveform-stereo-float32le-pcmwaveformat.wav"
    );
    std::shared_ptr<const VirtualFile> failingFile = std::make_shared<FailingVirtualFile>(
      file
    );

    // If the error is forwarded correctly, the domain_error will resurface from the call.
    // Should a plain runtime_error surface here, then error checking was happening but
    // the libwavpack error return took precedence over the VirtualFile exception, which is
    // not what we want because it obscures the root cause of the error.
    EXPECT_THROW(
      WaveformTrackDecoder decoder(failingFile),
      std::domain_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WaveformTrackDecoderTest, ReportsChannelOrderForExoticFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"waveform-exotic-int16le-waveformatextensible.wav"
    );

    WaveformTrackDecoder decoder(file);

    const std::vector<ChannelPlacement> &order = decoder.GetChannelOrder();
    ASSERT_EQ(order.size(), 2U);
    EXPECT_EQ(order[0], ChannelPlacement::BackRight);
    EXPECT_EQ(order[1], ChannelPlacement::SideLeft);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WaveformTrackDecoderTest, ReportsChannelOrderForSevenDotOneFile) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"waveform-7dot1-int16le-waveformatextensible.wav"
    );

    WaveformTrackDecoder decoder(file);

    const std::vector<ChannelPlacement> &order = decoder.GetChannelOrder();
    ASSERT_EQ(order.size(), 8U);
    EXPECT_EQ(order[0], ChannelPlacement::FrontLeft);
    EXPECT_EQ(order[1], ChannelPlacement::FrontRight);
    EXPECT_EQ(order[2], ChannelPlacement::FrontCenter);
    EXPECT_EQ(order[3], ChannelPlacement::LowFrequencyEffects);
    EXPECT_EQ(order[4], ChannelPlacement::BackLeft);
    EXPECT_EQ(order[5], ChannelPlacement::BackRight);
    EXPECT_EQ(order[6], ChannelPlacement::SideLeft);
    EXPECT_EQ(order[7], ChannelPlacement::SideRight);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WaveformTrackDecoderTest, DecodesFloatingPointDirect) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"waveform-stereo-float32le-pcmwaveformat.wav"
    );

    WaveformTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<float> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    TestAudioVerifier::VerifyStereo(samples, channelCount, 44100);
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  TEST(WavPackTrackDecoderTest, DecodesFloatingPointSeparated) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-float32-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();

    std::vector<float> leftSamples(frameCount);
    std::vector<float> rightSamples(frameCount);
    float *channels[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(channels, 0, frameCount);

    TestAudioVerifier::VerifyStereo(leftSamples, rightSamples, 44100);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes24BitQuantizedToFloat) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int24-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<float> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    TestAudioVerifier::VerifyStereo(samples, channelCount, 44100);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes24BitQuantizedToFloatSeparated) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int24-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();

    std::vector<float> leftSamples(frameCount);
    std::vector<float> rightSamples(frameCount);
    float *channels[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(channels, 0, frameCount);

    TestAudioVerifier::VerifyStereo(leftSamples, rightSamples, 44100);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes24BitTo16BitQuantized) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int24-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<std::int16_t> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    {
      std::vector<float> floatSamples(frameCount * channelCount);
      Processing::SampleConverter::Reconstruct(
        samples.data(), 16, floatSamples.data(), samples.size()
      );
      TestAudioVerifier::VerifyStereo(floatSamples, channelCount, 44100);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes24BitTo16BitQuantizedSeparated) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int24-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();

    std::vector<std::int16_t> leftSamples(frameCount);
    std::vector<std::int16_t> rightSamples(frameCount);
    std::int16_t *channels[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(channels, 0, frameCount);

    {
      std::vector<float> leftFloatSamples(frameCount);
      std::vector<float> rightFloatSamples(frameCount);
      Processing::SampleConverter::Reconstruct(
        leftSamples.data(), 16, leftFloatSamples.data(), frameCount
      );
      Processing::SampleConverter::Reconstruct(
        rightSamples.data(), 16, rightFloatSamples.data(), frameCount
      );
      TestAudioVerifier::VerifyStereo(leftFloatSamples, rightFloatSamples, 44100);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes16BitTo32BitQuantized) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int16-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();
    std::size_t channelCount = decoder.CountChannels();

    std::vector<std::int32_t> samples(frameCount * channelCount);
    decoder.DecodeInterleaved(samples.data(), 0, frameCount);

    {
      std::vector<float> floatSamples(frameCount * channelCount);
      Processing::SampleConverter::Reconstruct(
        samples.data(), 32, floatSamples.data(), samples.size()
      );
      TestAudioVerifier::VerifyStereo(floatSamples, channelCount, 44100);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WavPackTrackDecoderTest, Decodes16BitTo32BitQuantizedSeparated) {
    std::shared_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(
      GetResourcesDirectory() + u8"wavpack-stereo-int16-v416.wv"
    );

    WavPackTrackDecoder decoder(file);

    std::size_t frameCount = decoder.CountFrames();

    std::vector<std::int32_t> leftSamples(frameCount);
    std::vector<std::int32_t> rightSamples(frameCount);
    std::int32_t *channels[] = { leftSamples.data(), rightSamples.data() };
    decoder.DecodeSeparated(channels, 0, frameCount);

    {
      std::vector<float> leftFloatSamples(frameCount);
      std::vector<float> rightFloatSamples(frameCount);
      Processing::SampleConverter::Reconstruct(
        leftSamples.data(), 32, leftFloatSamples.data(), frameCount
      );
      Processing::SampleConverter::Reconstruct(
        rightSamples.data(), 32, rightFloatSamples.data(), frameCount
      );
      TestAudioVerifier::VerifyStereo(leftFloatSamples, rightFloatSamples, 44100);
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
