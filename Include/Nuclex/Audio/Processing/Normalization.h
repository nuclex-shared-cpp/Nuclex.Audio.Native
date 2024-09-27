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

#ifndef NUCLEX_AUDIO_PROCESSING_NORMALIZATION_H
#define NUCLEX_AUDIO_PROCESSING_NORMALIZATION_H

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

  /// <summary>Helper methods to normalize integers to floating point values</summary>
  /// <remarks>
  ///   <para>
  ///     This is a relatively tame wrapper for some SIMD instructions with fallbacks.
  ///     When reconstructing quantized signals, shifting them to the right before
  ///     converting them to floating point is also pretty common, so this class offers
  ///     a variant that does that.
  ///   </para>
  /// </remarks>
  class Normalization {

    /// <summary>Converts an integer into a normalized float by dividing it</summary>
    /// <param name="value">Value that will be normalized</param>
    /// <param name="quotient">Quotient by which the float value will be divided</param>
    /// <returns>The normalized floating point value</returns>
    public: static inline float DivideInt32ToFloat(
      const std::int32_t value, float quotient
    );

    /// <summary>Converts an integer into a normalized float by dividing it</summary>
    /// <param name="value">Value that will be normalized</param>
    /// <param name="quotient">Quotient by which the float value will be divided</param>
    /// <returns>The normalized floating point value</returns>
    public: static inline double DivideInt32ToFloat(
      const std::int32_t value, double quotient
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void DivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, float quotient, float *results/*[4]*/
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void DivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, double quotient, float *results/*[4]*/
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void DivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, double quotient, double *results/*[4]*/
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="shift">Number of bits by which the integers will be shifted</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void ShiftAndDivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, int shift, float quotient, float *results/*[4]*/
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="shift">Number of bits by which the integers will be shifted</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void ShiftAndDivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, int shift, double quotient, float *results/*[4]*/
    );

    /// <summary>Converts 4 integers into normalized floats by dividing them</summary>
    /// <param name="values">Array of 4 integer values that will be normalized</param>
    /// <param name="shift">Number of bits by which the integers will be shifted</param>
    /// <param name="quotient">Quotient by which the float values will be divided</param>
    /// <param name="results">Array that will receive the normalized float values</param>
    public: static inline void ShiftAndDivideInt32ToFloatx4(
      const std::int32_t *values/*[4]*/, int shift, double quotient, double *results/*[4]*/
    );

  };

  // ------------------------------------------------------------------------------------------- //

  inline float Normalization::DivideInt32ToFloat(
    const std::int32_t value, float quotient
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    return _mm_cvtss_f32(
      _mm_div_ps(_mm_set_ss(static_cast<float>(value)), _mm_set_ss(quotient))
    );
//#elif defined(__ARM_NEON)
#else
    return static_cast<float>(value) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline double Normalization::DivideInt32ToFloat(
    const std::int32_t value, double quotient
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    return _mm_cvtsd_f64(
      _mm_div_pd(_mm_set_sd(static_cast<float>(value)), _mm_set_sd(quotient))
    );
//#elif defined(__ARM_NEON)
#else
    return static_cast<float>(value) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::DivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, float quotient, float *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    _mm_storeu_ps(
      results,
      _mm_div_ps(
        _mm_cvtepi32_ps(
          _mm_loadu_si128(reinterpret_cast<const __m128i *>(values))
        ),
        _mm_set1_ps(quotient)
      )
    );
//#elif defined(__ARM_NEON)
#else
    results[0] = static_cast<float>(values[0]) / quotient;
    results[1] = static_cast<float>(values[1]) / quotient;
    results[2] = static_cast<float>(values[2]) / quotient;
    results[3] = static_cast<float>(values[3]) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::DivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, double quotient, float *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128i valuesVector = _mm_loadu_si128(reinterpret_cast<const __m128i *>(values));
    __m128d quotientVector = _mm_set1_pd(quotient);

    __m128 resultVector = _mm_cvtpd_ps(
      _mm_div_pd(
        _mm_cvtepi32_pd(_mm_unpackhi_epi64(valuesVector, valuesVector)),
        quotientVector
      )
    );
    _mm_storeu_ps(
      results,
      _mm_movelh_ps(
        _mm_cvtpd_ps(
          _mm_div_pd(
            _mm_cvtepi32_pd(valuesVector),
            quotientVector
          )
        ),
        resultVector
      )
    );
//#elif defined(__ARM_NEON)
#else
    //assert(quotient >= 0.0f);
    results[0] = static_cast<float>(static_cast<double>(values[0]) / quotient);
    results[1] = static_cast<float>(static_cast<double>(values[1]) / quotient);
    results[2] = static_cast<float>(static_cast<double>(values[2]) / quotient);
    results[3] = static_cast<float>(static_cast<double>(values[3]) / quotient);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::DivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, double quotient, double *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128i valuesVector = _mm_loadu_si128(reinterpret_cast<const __m128i *>(values));
    __m128d quotientVector = _mm_set1_pd(quotient);

    _mm_storeu_pd(
      results,
      _mm_div_pd(
        _mm_cvtepi32_pd(valuesVector),
        quotientVector
      )
    );
    _mm_storeu_pd(
      results + 2,
      _mm_div_pd(
        _mm_cvtepi32_pd(_mm_unpackhi_epi64(valuesVector, valuesVector)),
        quotientVector
      )
    );
//#elif defined(__ARM_NEON)
#else
    //assert(quotient >= 0.0f);
    results[0] = static_cast<double>(values[0]) / quotient;
    results[1] = static_cast<double>(values[1]) / quotient;
    results[2] = static_cast<double>(values[2]) / quotient;
    results[3] = static_cast<double>(values[3]) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::ShiftAndDivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, int shift, float quotient, float *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    _mm_storeu_ps(
      results,
      _mm_div_ps(
        _mm_cvtepi32_ps(
          _mm_srai_epi32(
            _mm_loadu_si128(reinterpret_cast<const __m128i *>(values)),
            shift
          )
        ),
        _mm_set1_ps(quotient)
      )
    );
//#elif defined(__ARM_NEON)
#else
    results[0] = static_cast<float>(values[0] >> shift) / quotient;
    results[1] = static_cast<float>(values[1] >> shift) / quotient;
    results[2] = static_cast<float>(values[2] >> shift) / quotient;
    results[3] = static_cast<float>(values[3] >> shift) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::ShiftAndDivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, int shift, double quotient, float *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128i valuesVector = _mm_srai_epi32(
      _mm_loadu_si128(reinterpret_cast<const __m128i *>(values)),
      shift
    );
    __m128d quotientVector = _mm_set1_pd(quotient);

    __m128 resultVector = _mm_cvtpd_ps(
      _mm_div_pd(
        _mm_cvtepi32_pd(_mm_unpackhi_epi64(valuesVector, valuesVector)),
        quotientVector
      )
    );
    _mm_storeu_ps(
      results,
      _mm_movelh_ps(
        _mm_cvtpd_ps(
          _mm_div_pd(
            _mm_cvtepi32_pd(valuesVector),
            quotientVector
          )
        ),
        resultVector
      )
    );
//#elif defined(__ARM_NEON)
#else
    results[0] = static_cast<float>(static_cast<double>(values[0] >> shift) / quotient);
    results[1] = static_cast<float>(static_cast<double>(values[1] >> shift) / quotient);
    results[2] = static_cast<float>(static_cast<double>(values[2] >> shift) / quotient);
    results[3] = static_cast<float>(static_cast<double>(values[3] >> shift) / quotient);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void Normalization::ShiftAndDivideInt32ToFloatx4(
    const std::int32_t *values/*[4]*/, int shift, double quotient, double *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_CVTS_AVAILABLE)
    __m128i valuesVector = _mm_srai_epi32(
      _mm_loadu_si128(reinterpret_cast<const __m128i *>(values)),
      shift
    );
    __m128d quotientVector = _mm_set1_pd(quotient);

    _mm_storeu_pd(
      results,
      _mm_div_pd(
        _mm_cvtepi32_pd(valuesVector),
        quotientVector
      )
    );
    _mm_storeu_pd(
      results + 2,
      _mm_div_pd(
        _mm_cvtepi32_pd(_mm_unpackhi_epi64(valuesVector, valuesVector)),
        quotientVector
      )
    );
//#elif defined(__ARM_NEON)
#else
    results[0] = static_cast<double>(values[0] >> shift) / quotient;
    results[1] = static_cast<double>(values[1] >> shift) / quotient;
    results[2] = static_cast<double>(values[2] >> shift) / quotient;
    results[3] = static_cast<double>(values[3] >> shift) / quotient;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_NORMALIZATION_H
