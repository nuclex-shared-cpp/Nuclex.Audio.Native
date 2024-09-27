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

//#undef NUCLEX_AUDIO_CVTS_AVAILABLE

// TODO: ARM support?
//#include <arm_neon.h>

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper methods to round floating point values to integers</summary>
  /// <remarks>
  ///   <para>
  ///     This is a relatively tame wrapper for some SIMD instructions with fallbacks.
  ///     When quantizing signals, multiplying them by a factor before converting to
  ///     integers is also pretty common, so this class offers a variant that does that.
  ///   </para>
  /// </remarks>
  class Rounding {

    /// <summary>Rounds a floating point value to the nearest integer</summary>
    /// <param name="value">Floating point value that will be rounded</param>
    /// <returns>The 32-bit integer narest to the floating point value</returns>
    /// <remarks>
    ///   This is a bit wasteful as it excutes a whole SIMD 4-operand instruction
    ///   for a single value, but we want all inputs to go through the exact same
    ///   code path and not risk <code>ffastmath</code> optimizations or somesuch
    ///   creating differently rounded results for single-value calculations.
    /// </remarks>
    public: static inline std::int32_t NearestInt32(float value);

    /// <summary>Rounds a floating point value to the nearest integer</summary>
    /// <param name="value">Floating point value that will be rounded</param>
    /// <returns>The 32-bit integer narest to the floating point value</returns>
    /// <remarks>
    ///   This is a bit wasteful as it excutes a whole SIMD 4-operand instruction
    ///   for a single value, but we want all inputs to go through the exact same
    ///   code path and not risk <code>ffastmath</code> optimizations or somesuch
    ///   creating differently rounded results for single-value calculations.
    /// </remarks>
    public: static inline std::int32_t NearestInt32(double value);

    /// <summary>Rounds 4 floating point values to their nearest integers</summary>
    /// <param name="values">Array of 4 floating point values that will be rounded</param>
    /// <param name="results">Receives the 4 integers nearest to the float values</param>
    public: static inline void NearestInt32x4(
      const float *values/*[4]*/, std::int32_t *results/*[4]*/
    );

    /// <summary>Rounds 4 floating point values to the nearest integers</summary>
    /// <param name="values">Array of 4 floating point values that will be rounded</param>
    /// <param name="results">Receives the 4 integers nearest to the float values</param>
    public: static inline void NearestInt32x4(
      const double *values/*[4]*/, std::int32_t *results/*[4]*/
    );

    /// <summary>Multiplies and rounds 4 floating point values to the nearest integers</summary>
    /// <param name="values">Array of 4 floating point values that will be rounded</param>
    /// <param name="factor">Factor by which the floating point values will be multiplied</param>
    /// <param name="results">Receives the 4 integers nearest to the float values</param>
    public: static inline void MultiplyToNearestInt32x4(
      const float *values/*[4]*/, float factor, std::int32_t *results/*[4]*/
    );

    /// <summary>Multiplies and rounds 4 floating point values to the nearest integers</summary>
    /// <param name="values">Array of 4 floating point values that will be rounded</param>
    /// <param name="factor">Factor by which the floating point values will be multiplied</param>
    /// <param name="results">Receives the 4 integers nearest to the float values</param>
    public: static inline void MultiplyToNearestInt32x4(
      const float *values/*[4]*/, double factor, std::int32_t *results/*[4]*/
    );

    /// <summary>Multiplies and rounds 4 floating point values to the nearest integers</summary>
    /// <param name="values">Array of 4 floating point values that will be rounded</param>
    /// <param name="factor">Factor by which the floating point values will be multiplied</param>
    /// <param name="results">Receives the 4 integers nearest to the float values</param>
    public: static inline void MultiplyToNearestInt32x4(
      const double *values/*[4]*/, double factor, std::int32_t *results/*[4]*/
    );

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

  inline void Rounding::NearestInt32x4(
    const float *values/*[4]*/, std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_cvtps_epi32(_mm_loadu_ps(values))
    );
//#elif defined(__ARM_NEON)
//    float32x4_t v = vld1q_f32(values);
//    int32x4_t r = vcvtq_s32_f32(v);
//    vst1q_s32(results, r);
#else
    results[0] = static_cast<std::int32_t>(values[0] + std::copysign(0.5f, values[0]));
    results[1] = static_cast<std::int32_t>(values[1] + std::copysign(0.5f, values[1]));
    results[2] = static_cast<std::int32_t>(values[2] + std::copysign(0.5f, values[2]));
    results[3] = static_cast<std::int32_t>(values[3] + std::copysign(0.5f, values[3]));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Rounding::NearestInt32x4(
    const double *values/*[4]*/, std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_unpacklo_epi64(
        _mm_cvtpd_epi32(_mm_loadu_pd(values)),
        _mm_cvtpd_epi32(_mm_loadu_pd(values + 2))
      )
    );
//#elif defined(__ARM_NEON)
//    float64x2_t v_low = vld1q_f64(&values[0]);
//    float64x2_t v_high = vld1q_f64(&values[2]);
//    int32x4_t r = vcombine_s32(vcvtq_s32_f64(v_low), vcvtq_s32_f64(v_high));
//    vst1q_s32(results, r);
#else
    results[0] = static_cast<std::int32_t>(values[0] + std::copysign(0.5, values[0]));
    results[1] = static_cast<std::int32_t>(values[1] + std::copysign(0.5, values[1]));
    results[2] = static_cast<std::int32_t>(values[2] + std::copysign(0.5, values[2]));
    results[3] = static_cast<std::int32_t>(values[3] + std::copysign(0.5, values[3]));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Rounding::MultiplyToNearestInt32x4(
    const float *values/*[4]*/, float factor, std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_cvtps_epi32(
        _mm_mul_ps(
          _mm_loadu_ps(values),
          _mm_set1_ps(factor)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    //assert(factor >= 0.0f);
    results[0] = static_cast<std::int32_t>(values[0] * factor + std::copysign(0.5f, values[0]));
    results[1] = static_cast<std::int32_t>(values[1] * factor + std::copysign(0.5f, values[1]));
    results[2] = static_cast<std::int32_t>(values[2] * factor + std::copysign(0.5f, values[2]));
    results[3] = static_cast<std::int32_t>(values[3] * factor + std::copysign(0.5f, values[3]));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Rounding::MultiplyToNearestInt32x4(
    const float *values/*[4]*/, double factor, std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128 valuesVector = _mm_loadu_ps(values);
    __m128d factorVector = _mm_set1_pd(factor);

    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_unpacklo_epi64(
        _mm_cvtpd_epi32(
          _mm_mul_pd(
            _mm_cvtps_pd(valuesVector), // loads the lower 2 values
            factorVector
          )
        ),
        _mm_cvtpd_epi32(
          _mm_mul_pd(
            _mm_cvtps_pd(_mm_movehl_ps(valuesVector, valuesVector)), // loads the upper 2 values
            factorVector
          )
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    //assert(factor >= 0.0f);
    results[0] = static_cast<std::int32_t>(
      static_cast<double>(values[0]) * factor + std::copysign(0.5, values[0])
    );
    results[1] = static_cast<std::int32_t>(
      static_cast<double>(values[1]) * factor + std::copysign(0.5, values[1])
    );
    results[2] = static_cast<std::int32_t>(
      static_cast<double>(values[2]) * factor + std::copysign(0.5, values[2])
    );
    results[3] = static_cast<std::int32_t>(
      static_cast<double>(values[3]) * factor + std::copysign(0.5, values[3])
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Rounding::MultiplyToNearestInt32x4(
    const double *values/*[4]*/, double factor, std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128d factorVector = _mm_set1_pd(factor);

    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_unpacklo_epi64(
        _mm_cvtpd_epi32(
          _mm_mul_pd(
            _mm_loadu_pd(values),
            factorVector
          )
        ),
        _mm_cvtpd_epi32(
          _mm_mul_pd(
            _mm_loadu_pd(values + 2),
            factorVector
          )
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    //assert(factor >= 0.0f);
    results[0] = static_cast<std::int32_t>(values[0] * factor + std::copysign(0.5, values[0]));
    results[1] = static_cast<std::int32_t>(values[1] * factor + std::copysign(0.5, values[1]));
    results[2] = static_cast<std::int32_t>(values[2] * factor + std::copysign(0.5, values[2]));
    results[3] = static_cast<std::int32_t>(values[3] * factor + std::copysign(0.5, values[3]));
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_ROUNDING_H
