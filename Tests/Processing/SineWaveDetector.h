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

#ifndef NUCLEX_AUDIO_PROCESSING_SINEWAVEDETECTOR_H
#define NUCLEX_AUDIO_PROCESSING_SINEWAVEDETECTOR_H

#include "Nuclex/Audio/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks if a signal matches a sine wave</summary>
  class SineWaveDetector {

    /// <summary>Initialies a new sine wave detector</summary>
    public: SineWaveDetector();

    /// <summary>Tries to figure out the amplitude of the signal</summary>
    /// <param name="samples">Samples from which the amplitude will be detected</param>
    /// <param name="sampleCount">Number of samples that have been provided</param>
    /// <remarks>
    ///   This merely looks at the signal's peaks in either direction. If this wasn't 
    ///   a small helper class for unit tests, there are fourier-based methods or you
    ///   could do a curve fit on the sine wave.
    /// </remarks>
    public: void DetectAmplitude(const float *samples, std::size_t sampleCount);

    /// <summary>Adds samples to be processed by the sine wave detector</summary>
    /// <param name="samples">Samples that will be processed by the detector</param>
    /// <param name="sampleCount">Number of samples that should be processed</param>
    public: void AddSamples(const float *samples, std::size_t sampleCount);

    /// <summary>Processes a single sample</summary?
    /// <param name="sample">Sample that will be processed</param>
    public: void AddSample(float sample);

    /// <summary>Returns the assumed or detected amplitude of the signal</summary>
    /// <returns>The assumed or detected amplitude of the signal</returns>
    public: float GetAmplitude() const;

    /// <summary>Calculates the deviation from a pure sine wave per sample</summary>
    /// <returns>The deviation from a pure sine wave per sample</returns>
    public: float GetError() const;

    /// <summary>Returns the frequency of the signal in samples per second</summary>
    /// <param name="sampleRate">Sample rate of the signal</param>
    /// <returns>The frequency of the signal</returns>
    public: float GetFrequency(float sampleRate) const;

    /// <summary>Assumed amplitude of the signal</summary>
    private: float amplitude;
    /// <summary>Number of samples that have been processed so far</summary>
    private: std::size_t sampleCount;
    /// <summary>Sample that has been processed previously</summary>
    private: float previousSample;
    /// <summary>Sample from which on we began analyzing the signal</summary>
    private: std::size_t startSampleIndex;
    /// <summary>Angle that was determined for the start sample</summary>
    private: double startAngle;
    /// <summary>Angle progression accumulated since the start sample</summary>
    private: double accumulatedAngle;
    /// <summary>Whether the previous sample was a rising sample</summary>
    private: bool previousWasFalling;
    /// <summary>Determined angle of the previous sample</summary>
    private: double previousAngle;

    /// <summary>Accumulated mismatch from a pure sine wave</summary>
    private: double accumulatedError;
    /// <summary>How many times the curve has crossed the zero line</summary>
    private: std::size_t zeroCrossingCount;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_SINEWAVEDETECTOR_H
