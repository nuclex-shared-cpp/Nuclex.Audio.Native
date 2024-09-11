#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "./SineWaveDetector.h"

#include <cmath> // for std::asin(), std::min()
#include <optional> // for std::optional
#include <vector> // for std::vector
#include <stdexcept> // for std::runtime_error
#include <algorithm> // for std::max_element()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>The mathematical constant 'PI' as a double, nothing less</summary>
  constexpr double pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406;

  /// <summary>Equivalent to 2xPI, one full turn in radian</summary>
  constexpr double tau = 6.28318530717958647692528676655900576839433879875021164194988918461563281;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Normalizes an angle into the -180 to +180 range</summary>
  /// <param name="angle">Angle that will be normalized</param>
  /// <returns>The normalized angle</returns>
  double getNormalizedAngle(double angle) {
    return std::fmod(std::fmod(angle + pi, tau) + tau, tau) - pi;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the forward delta angle between two angles</summary>
  /// <param name="angle1">First angle from which on to calculate the delta</param>
  /// <param name="angle2">Second angle towards which to calculate the delta</param>
  /// <returns>
  ///   The amount <paramref ref="angle1" /> needs to be rotated forward to
  ///   match the angle <paramref ref="angle2" />
  /// </returns>
  double getForwardDeltaAngle(double angle1, double angle2) {
    return std::fmod(std::fmod(angle2 - angle1, tau) + tau, tau);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Clamps a value to the -1.0 .. +1.0 range</summary>
  /// <param name="value">Value that will be clamped</param>
  /// <returns>The value in the -1.0 .. +1.0 range</returns>
  double clampToPlusMinusOne(double value) {
    return std::min(std::max(value, -1.0), 1.0);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  SineWaveDetector::SineWaveDetector() :
    amplitude(1.0f),
    sampleCount(0),
    previousSample(0.0f),
    startSampleIndex(std::size_t(-1)),
    startAngle(0.0),
    accumulatedAngle(0.0),
    previousWasFalling(false),
    accumulatedError(0.0) {}

  // ------------------------------------------------------------------------------------------- //

  void SineWaveDetector::DetectAmplitude(
    const float *samples, std::size_t sampleCount, std::size_t channelCount /* = 1 */
  ) {
    std::vector<float> upperPeaks, lowerPeaks;

    std::size_t indexOfLastDoubleClimb = std::size_t(-1);
    std::size_t indexOfLastDoubleFall = std::size_t(-1);
    bool previousWasRising = false, previousWasFalling = false;

    // This is just scanning for the highest and lowest points in the signal,
    // but - to counteract dither and/or compression jitter - it will only consider
    // peaks frames by two successive climbing samples and two successive falling samples.
    float previous = samples[0];
    for(std::size_t index = channelCount; index < sampleCount; index += channelCount) {
      float current = samples[index];
      if(previous < current) {
        if(previousWasRising) {
          indexOfLastDoubleClimb = index;
          if(indexOfLastDoubleFall != std::size_t(-1)) {
            lowerPeaks.push_back(
              *std::min_element(
                samples + indexOfLastDoubleFall,
                samples + indexOfLastDoubleClimb
              )
            );
            indexOfLastDoubleClimb = std::size_t(-1);
            indexOfLastDoubleFall = std::size_t(-1);
          }
        }
        previousWasRising = true;
        previousWasFalling = false;
      } else if(current < previous) {
        if(previousWasFalling) {
          indexOfLastDoubleFall = index;
          if(indexOfLastDoubleClimb != std::size_t(-1)) {
            upperPeaks.push_back(
              *std::max_element(
                samples + indexOfLastDoubleClimb,
                samples + indexOfLastDoubleFall
              )
            );
            indexOfLastDoubleFall = std::size_t(-1);
            indexOfLastDoubleClimb = std::size_t(-1);
          }
        }
        previousWasFalling = true;
        previousWasRising = false;
      }
      previous = current;
    }

    // If no peaks or troughs were found, we have nothing to work on
    if(upperPeaks.empty() || lowerPeaks.empty()) {
      throw std::runtime_error(u8"Not enough data to detect peaks or data is too random");
    }

    // Extract the average upper peak
    double upperPeak;
    {
      upperPeak = static_cast<double>(upperPeaks[0]);
      for(std::size_t index = 1; index < upperPeaks.size(); ++index) {
        upperPeak += static_cast<double>(upperPeaks[index]);
      }
      upperPeak /= static_cast<double>(upperPeaks.size());
    }

    // Extract the average lower peak (trough)
    double lowerPeak;
    {
      lowerPeak = static_cast<double>(lowerPeaks[0]);
      for(std::size_t index = 1; index < lowerPeaks.size(); ++index) {
        lowerPeak += static_cast<double>(lowerPeaks[index]);
      }
      lowerPeak /= static_cast<double>(lowerPeaks.size());
    }

    // Now calculate the amplitude given the two peaks
    this->amplitude = static_cast<float>((upperPeak - lowerPeak) / 2.0);
  }

  // ------------------------------------------------------------------------------------------- //

  void SineWaveDetector::AddSamples(
    const float *samples, std::size_t sampleCount, std::size_t channelCount /* = 1 */
  ) {
    for(std::size_t index = 0; index < sampleCount; index += channelCount) {
      AddSample(samples[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void SineWaveDetector::AddSample(float sample) {
    ++this->sampleCount;

    // Stage 1: Just record the first and second sample, we can't determine anything yet
    if(this->sampleCount == 1) {
      this->previousSample = sample;
      return;
    }
    if(this->sampleCount == 2) {
      this->previousWasFalling = (sample < this->previousSample);
      this->previousSample = sample;
      return;
    }

    // Stage 2: Consume samples until we are certain which flank we're on
    if(this->startSampleIndex == std::size_t(-1)) {
      std::optional<bool> isInsideFallingHalf;
      if(sample < this->previousSample) {
        if(this->previousWasFalling) {
          isInsideFallingHalf = true;
        }
        this->previousWasFalling = true;
      } else if(this->previousSample < sample) {
        if(!this->previousWasFalling) {
          isInsideFallingHalf = false;
        }
        this->previousWasFalling = false;
      }

      if(isInsideFallingHalf.has_value()) {
        double halfWaveAngle = std::asin(clampToPlusMinusOne(sample / this->amplitude));
        if(isInsideFallingHalf.value()) {
          this->startAngle = this->previousAngle = pi - halfWaveAngle;
        } else {
          this->startAngle = this->previousAngle = halfWaveAngle;
        }
        this->startSampleIndex = this->sampleCount;
      }

      return; // return even when setting the start sample index
    }

    double angle;
    {
      double halfWaveAngle = std::asin(clampToPlusMinusOne(sample / this->amplitude));
      double forwardDelta = getForwardDeltaAngle(this->previousAngle, halfWaveAngle);

      double inverseAngle = pi - halfWaveAngle;
      double inverseForwardDelta = getForwardDeltaAngle(this->previousAngle, inverseAngle);

      if(inverseForwardDelta < forwardDelta) {
        angle = getNormalizedAngle(inverseAngle);
      } else {
        angle = getNormalizedAngle(halfWaveAngle);
      }
    }

    // Stage 3: Verify that the signal is advancing on a sine curve at constant speed
    {
      double expectedAngle;
      {
        std::size_t averagedSampleCount = this->sampleCount - this->startSampleIndex - 1;
        if(averagedSampleCount == 0) {
          expectedAngle = angle; // with only 1 sample, the average is clear :-)
        } else {
          double averageStep = this->accumulatedAngle / averagedSampleCount;
          expectedAngle = this->startAngle + this->accumulatedAngle + averageStep;
        }
      }

      this->accumulatedAngle += getForwardDeltaAngle(this->previousAngle, angle);
      this->previousAngle = angle;

      float expectedSample = std::sin(expectedAngle) * this->amplitude;
      this->accumulatedError += std::fabs(expectedSample - sample);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  float SineWaveDetector::GetAmplitude() const {
    return this->amplitude;
  }

  // ------------------------------------------------------------------------------------------- //

  float SineWaveDetector::GetError() const {
    if(this->startSampleIndex == std::size_t(-1)) {
      return 0.0;
    }

    std::size_t averagedSampleCount = this->sampleCount - this->startSampleIndex - 1;
    if(averagedSampleCount == 0) {
      return 0.0;
    }

    return static_cast<float>(
      (this->accumulatedError / averagedSampleCount) * this->amplitude
    );
  }

  // ------------------------------------------------------------------------------------------- //

  float SineWaveDetector::GetFrequency(float sampleRate) const {
    if(this->startSampleIndex == std::size_t(-1)) {
      return 0.0;
    }

    std::size_t accumulatedSampleCount = this->sampleCount - this->startSampleIndex - 1;
    if(accumulatedSampleCount == 0) {
      return 0.0;
    }

    double zeroCrossings = (this->accumulatedAngle / pi);
    double signalLength = static_cast<double>(accumulatedSampleCount) / sampleRate;

    return static_cast<float>(zeroCrossings / signalLength / 2.0); // frequency is full waves
  }

  // ------------------------------------------------------------------------------------------- //

  float SineWaveDetector::GetPhase() const {
    std::size_t averagedSampleCount = this->sampleCount - this->startSampleIndex - 1;
    double averageStep = this->accumulatedAngle / averagedSampleCount;

    return static_cast<float>(
      this->startAngle - (this->startSampleIndex * averageStep)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  float SineWaveDetector::GetPhase360() const {
    std::size_t averagedSampleCount = this->sampleCount - this->startSampleIndex - 1;
    double averageStep = this->accumulatedAngle / averagedSampleCount;

    return static_cast<float>(
      (this->startAngle - (this->startSampleIndex * averageStep)) * 180.0 / pi
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing
