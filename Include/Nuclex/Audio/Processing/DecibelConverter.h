#pragma once
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

#ifndef NUCLEX_AUDIO_PROCESSING_DECIBELCONVERTER_H
#define NUCLEX_AUDIO_PROCESSING_DECIBELCONVERTER_H

#include "Nuclex/Audio/Config.h"

#include <limits> // for std::numeric_limits
#include <cmath> // for std::sgn

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods to calculate decibels from amplitude</summary>
  class DecibelConverter {

    /// <summary>Converts a linear amplitude value into decibel</summary>
    /// <param name="amplitude">Linear amplitude that will be converted</param>
    /// <returns>The decibels relative to a normalized amplitude range</returns>
    /// <remarks>
    ///   Decibel is a relative, logarithmic scale. A popular usage of decibel for audio
    ///   is to state the distance from the signal ceiling. Negative decibel values mean
    ///   that the signal is below the ceiling, a positive decibel value would mean that
    ///   the signal is clipping. This value range appears automatically for a normalized
    ///   signal because an amplitude 1.0 translates to 0.0 decibels, anyhting below 1.0
    ///   becomes a negative decibel value and anythign above 1.0 becomes a positive value.
    /// </remarks>
    public: static inline float FromLinearAmplitude(float amplitude);

    /// <summary>Converts a linear amplitude value into decibel</summary>
    /// <param name="amplitude">Linear amplitude that will be converted</param>
    /// <returns>The decibels relative to a normalized amplitude range</returns>
    /// <remarks>
    ///   Decibel is a relative, logarithmic scale. A popular usage of decibel for audio
    ///   is to state the distance from the signal ceiling. Negative decibel values mean
    ///   that the signal is below the ceiling, a positive decibel value would mean that
    ///   the signal is clipping. This value range appears automatically for a normalized
    ///   signal because an amplitude 1.0 translates to 0.0 decibels, anyhting below 1.0
    ///   becomes a negative decibel value and anythign above 1.0 becomes a positive value.
    /// </remarks>
    public: static inline double FromLinearAmplitude(double amplitude);

  };

  // ------------------------------------------------------------------------------------------- //

  inline float DecibelConverter::FromLinearAmplitude(float amplitude) {
    amplitude = std::abs(amplitude);
    if(amplitude < std::numeric_limits<float>::epsilon()) {
      return std::numeric_limits<float>::infinity();
    } else {
      return std::log10(amplitude) * 20.0f;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline double DecibelConverter::FromLinearAmplitude(double amplitude) {
    amplitude = std::abs(amplitude);
    if(amplitude < std::numeric_limits<double>::epsilon()) {
      return std::numeric_limits<double>::infinity();
    } else {
      return std::log10(amplitude) * 20.0;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_DECIBELCONVERTER_H
