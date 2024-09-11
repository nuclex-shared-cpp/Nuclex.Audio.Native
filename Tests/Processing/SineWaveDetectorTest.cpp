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

#include "./SineWaveDetector.h"

#include <gtest/gtest.h>
#include <cmath>
#include <random>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The mathematical constant 'PI' as a double, nothing less</summary>
  constexpr double pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates a pure sine wave signal with the specified amplitude</summary>
  /// <param name="amplitude">Amplitude the generated signal will have</param>
  /// <returns>A vector containing the samples of a pure sine wave signal</returns>
  /// <remarks>
  ///   Assumes the sample rate of the signal is 48 KHz and generates a signal that
  ///   describes one full wave per 96 samples, resulting in a 500 Hz tone.
  /// </remarks>
  std::vector<float> generateSineWave(float amplitude) {
    std::vector<float> samples;
    samples.reserve(4800);

    for(std::size_t index = 0; index < 4800; ++index) {
      samples.push_back(
        static_cast<float>(
          std::sin(static_cast<double>(index) / 48.0 * pi) * amplitude
        )
      );
    }

    return samples;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates a random signal with the specified amplitude</summary>
  /// <param name="amplitude">Amplitude the generated signal will have</param>
  /// <returns>A vector containing the samples of a random signal</returns>
  std::vector<float> generateRandomWave(float amplitude) {
    std::vector<float> samples;
    samples.reserve(4800);

    {
      std::mt19937 randomNumberGenerator;
      std::uniform_real_distribution<float> floatDistribution(-amplitude, amplitude);

      for (std::size_t index = 0; index < 4800; ++index) {
        samples.push_back(floatDistribution(randomNumberGenerator));
      }
    }

    return samples;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  TEST(SineWaveDetectorTest, CanDeterminePeaks) {
    SineWaveDetector detector;

    std::vector<float> samples = generateSineWave(2.5f);
    detector.DetectAmplitude(samples.data(), samples.size());

    EXPECT_GE(detector.GetAmplitude(), 2.4f);
    EXPECT_LT(detector.GetAmplitude(), 2.6f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SineWaveDetectorTest, PureSineWaveHasLowError) {
    SineWaveDetector detector;

    std::vector<float> samples = generateSineWave(2.5f);
    detector.DetectAmplitude(samples.data(), samples.size());
    detector.AddSamples(samples.data(), samples.size());

    EXPECT_LT(detector.GetError(), 0.001f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SineWaveDetectorTest, CanEstimateFrequencyOfPureSineWave) {
    SineWaveDetector detector;

    std::vector<float> samples = generateSineWave(2.5f);
    detector.DetectAmplitude(samples.data(), samples.size());
    detector.AddSamples(samples.data(), samples.size());

    EXPECT_GE(detector.GetFrequency(48000.0f), 499.0f);
    EXPECT_LT(detector.GetFrequency(48000.0f), 501.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SineWaveDetectorTest, RandomSignalHasHighError) {
    SineWaveDetector detector;

    std::vector<float> samples = generateRandomWave(2.5f);
    detector.DetectAmplitude(samples.data(), samples.size());
    detector.AddSamples(samples.data(), samples.size());

    EXPECT_GE(detector.GetError(), 1.0f);

    EXPECT_GE(detector.GetFrequency(48000.0f), 500.0f);

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
