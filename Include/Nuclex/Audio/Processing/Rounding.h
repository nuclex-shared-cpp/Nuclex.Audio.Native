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

#ifndef NUCLEX_AUDIO_PROCESSING_ROUNDING_H
#define NUCLEX_AUDIO_PROCESSING_ROUNDING_H

#include "Nuclex/Audio/Config.h"

#include <cstdint> // for std::int32_t
#include <cmath> // for std::sgn

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
#include <x86intrin.h>
#endif

// Whether the cvts CPU instruction is supported by the targeted architecture
#if defined(_MSC_VER)
  // All SSE2 CPUs have the CVTS instructions, so if we're compiling for SSE2
  // or compiling to amd64 (which always has SSE2), they're available.
  #if defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
    #define NUCLEX_AUDIO_CVTS_AVAILABLE 1
  #endif
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  // Same logic on GCC and clang, except they declare availability of SSE2
  // explicitly, so we don't need to study architectures and implied features.
  #if defined(__SSE2__) || defined(__SSE2_MATH__)
    #define NUCLEX_AUDIO_CVTS_AVAILABLE 1
  #endif
#endif

// TODO: ARM support?
//#include <arm_neon.h>

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods to round floating point values to integers</summary>
  class Rounding {

    /// <summary>Rounds a floating point value to the nearest integer</summary>
    /// <param name="value">Floating point value that will be rounded</param>
    /// <returns>The nearest 32-bit integer to the floating point value</returns>
    public: static inline std::int32_t NearestInt32(float value);

    /// <summary>Rounds a floating point value to the nearest integer</summary>
    /// <param name="value">Floating point value that will be rounded</param>
    /// <returns>The nearest 32-bit integer to the floating point value</returns>
    public: static inline std::int32_t NearestInt32(double value);

  };

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t Rounding::NearestInt32(float value) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    return _mm_cvtss_si32(_mm_set_ss(value));
//#elif defined(__ARM_NEON)
//    return vgetq_lane_s32(vcvtq_s32_f32(vdupq_n_f32(value)), 0);
#else
    return static_cast<std::int32_t>(value + std::copysign(0.5f, value));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t Rounding::NearestInt32(double value) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    return _mm_cvtsd_si32(_mm_set_sd(value));
//#elif defined(__ARM_NEON)
//    return vgetq_lane_s32(vcvtq_s32_f64(vdupq_n_f64(value)), 0);
#else
    return static_cast<std::int32_t>(value + std::copysign(0.5, value));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_ROUNDING_H
