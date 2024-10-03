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

#include "./TestAudioVerifier.h"
#include "../Processing/SineWaveDetector.h"
#include "../ExpectRange.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  void TestAudioVerifier::VerifyStereo(
    const std::vector<float> &samples, std::size_t channelCount,
    std::size_t sampleRate /* = 48000 */
  ) {
    std::size_t frameCount = samples.size() / channelCount;

    ASSERT_EQ(channelCount, 2U);

    // Left signal should be at 0° phase, 25 Hz and have an amplitude of 1.0
    {
      Nuclex::Audio::Processing::SineWaveDetector left;
      left.DetectAmplitude(samples.data(), frameCount, channelCount);
      left.AddSamples(samples.data(), frameCount, channelCount);

      EXPECT_RANGE(left.GetFrequency(sampleRate), 24.9f, 25.1f);
      EXPECT_RANGE(left.GetAmplitude(), 0.9f, 1.1f);
      EXPECT_RANGE(left.GetPhase360(), -0.5f, 0.5f);
      EXPECT_LT(left.GetError(), 0.0001f);
    }

    // Right signal should be at 180° phase, 25 Hz and have an amplitude of 1.0
    {
      Nuclex::Audio::Processing::SineWaveDetector right;
      right.DetectAmplitude(samples.data() + 1, frameCount, channelCount);
      right.AddSamples(samples.data() + 1, frameCount, channelCount);

      EXPECT_RANGE(right.GetFrequency(sampleRate), 24.9f, 25.1f);
      EXPECT_RANGE(right.GetAmplitude(), 0.9f, 1.1f);
      EXPECT_RANGE(right.GetPhase360(), 179.5f, 180.5f); // or -180.0 .. -179.5...
      EXPECT_LT(right.GetError(), 0.0001f);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void TestAudioVerifier::VerifyStereo(
    const std::vector<float> &leftSamples, const std::vector<float> &rightSamples,
    std::size_t sampleRate /* = 48000 */
  ) {
    std::size_t frameCount = leftSamples.size();

    ASSERT_EQ(leftSamples.size(), rightSamples.size());

    // Left signal should be at 0° phase, 25 Hz and have an amplitude of 1.0
    {
      Nuclex::Audio::Processing::SineWaveDetector left;
      left.DetectAmplitude(leftSamples.data(), frameCount);
      left.AddSamples(leftSamples.data(), frameCount);

      EXPECT_RANGE(left.GetFrequency(sampleRate), 24.9f, 25.1f);
      EXPECT_RANGE(left.GetAmplitude(), 0.9f, 1.1f);
      EXPECT_RANGE(left.GetPhase360(), -0.5f, 0.5f);
      EXPECT_LT(left.GetError(), 0.0001f);
    }

    // Right signal should be at 180° phase, 25 Hz and have an amplitude of 1.0
    {
      Nuclex::Audio::Processing::SineWaveDetector right;
      right.DetectAmplitude(rightSamples.data(), frameCount);
      right.AddSamples(rightSamples.data(), frameCount);

      EXPECT_RANGE(right.GetFrequency(sampleRate), 24.9f, 25.1f);
      EXPECT_RANGE(right.GetAmplitude(), 0.9f, 1.1f);
      EXPECT_RANGE(right.GetPhase360(), 179.5f, 180.5f); // or -180.0 .. -179.5...
      EXPECT_LT(right.GetError(), 0.0001f);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
