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

#ifndef NUCLEX_AUDIO_PROCESSING_BITEXTENSION_H
#define NUCLEX_AUDIO_PROCESSING_BITEXTENSION_H

#include "Nuclex/Audio/Config.h"

#include <cstdint> // for std::int32_t
#include <cmath> // for std::sgn

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
#include <x86intrin.h>
#endif

// Whether the SSE2 SIMD instructions are supported by the targeted architecture
#if defined(_MSC_VER)
  // All SSE2 CPUs have the CVTS instructions, so if we're compiling for SSE2
  // or compiling to amd64 (which always has SSE2), they're available.
  #if defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
    #define NUCLEX_AUDIO_HAVE_SSE2 1
  #endif
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  // Same logic on GCC and clang, except they declare availability of SSE2
  // explicitly, so we don't need to study architectures and implied features.
  #if defined(__SSE2__) || defined(__SSE2_MATH__)
    #define NUCLEX_AUDIO_HAVE_SSE2 1
  #endif
#endif

//#undef NUCLEX_AUDIO_HAVE_SSE2

// TODO: ARM support?
//#include <arm_neon.h>

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper methods to repeat the bit patterns of integers to extend them</summary>
  /// <remarks>
  ///   <para>
  ///     When you want to turn a 16-bit value into a 24-bit one, for example, just padding
  ///     it with zero bits robs it of some of its range. By repeating the bit patterns
  ///     the value fills the entire range evenly, exactly like it would if it had been
  ///     converted to floating point and back to the higher-range integer.
  ///   </para>
  /// </remarks>
  class BitExtension {

    /// <summary>Repeats the specified number of bits in a signed integer</summary>
    /// <param name="value">Integer in which bits will be repeated</param>
    /// <param name="shift">Number of bits to shift to the right</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting</param>
    /// <returns>The 32-bit integer with the specified number of bits repeated</returns>
    public: static inline std::int32_t RepeatSigned(
      std::int32_t value, int shift, std::int32_t mask
    );

    /// <summary>Repeats the specified number of bits in a signed integer</summary>
    /// <param name="preShift">Number of bits to shift to the left first</param>
    /// <param name="value">Integer in which bits will be repeated</param>
    /// <param name="shift">Number of bits to shift to the right</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting</param>
    /// <returns>The 32-bit integer with the specified number of bits repeated</returns>
    public: static inline std::int32_t ShiftAndRepeatSigned(
      int preShift, std::int32_t value, int shift, std::int32_t mask
    );

    /// <summary>Triples the specified number of bits in a signed integer</summary>
    /// <param name="value">Integer in which bits will be tripled</param>
    /// <param name="shift">Number of bits to shift to the right (twice)</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting once</param>
    /// <returns>The 32-bit integer with the specified number of bits tripled</returns>
    public: static inline std::int32_t TripleSigned(
      std::int32_t value, int shift, std::int32_t mask
    );

    /// <summary>Triples the specified number of bits in a signed integer</summary>
    /// <param name="preShift">Number of bits to shift to the left first</param>
    /// <param name="value">Integer in which bits will be tripled</param>
    /// <param name="shift">Number of bits to shift to the right (twice)</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting once</param>
    /// <returns>The 32-bit integer with the specified number of bits tripled</returns>
    public: static inline std::int32_t ShiftAndTripleSigned(
      int preShift, std::int32_t value, int shift, std::int32_t mask
    );

    /// <summary>Repeats the specified number of bits in 4 signed integers</summary>
    /// <param name="values">Integers in which bits will be repeated</param>
    /// <param name="shift">Number of bits to shift to the right</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting</param>
    /// <param name="results">Receives the integers with repeated bits</param>
    public: static inline void RepeatSignedx4(
      const std::int32_t *values/*[4]*/,
      int shift, std::int32_t mask,
      std::int32_t *results/*[4]*/
    );

    /// <summary>Repeats the specified number of bits in 4 signed integers</summary>
    /// <param name="preShift">Number of bits to shift to the left first</param>
    /// <param name="values">Integers in which bits will be repeated</param>
    /// <param name="shift">Number of bits to shift to the right</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting</param>
    /// <param name="results">Receives the integers with repeated bits</param>
    public: static inline void ShiftAndRepeatSignedx4(
      int preShift,
      const std::int32_t *values/*[4]*/,
      int shift, std::int32_t mask,
      std::int32_t *results/*[4]*/
    );

    /// <summary>Triples the specified number of bits in 4 signed integers</summary>
    /// <param name="values">Integers in which bits will be tripled</param>
    /// <param name="shift">Number of bits to shift to the right (twice)</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting once</param>
    /// <param name="results">Receives the integers with tripled bits</param>
    public: static inline void TripleSignedx4(
      const std::int32_t *values/*[4]*/,
      int shift, std::int32_t mask,
      std::int32_t *results/*[4]*/
    );

    /// <summary>Triples the specified number of bits in 4 signed integers</summary>
    /// <param name="preShift">Number of bits to shift to the left first</param>
    /// <param name="values">Integers in which bits will be tripled</param>
    /// <param name="shift">Number of bits to shift to the right (twice)</param>
    /// <param name="mask">Mask of the bits to keep after right-shifting once</param>
    /// <param name="results">Receives the integers with tripled bits</param>
    public: static inline void ShiftAndTripleSignedx4(
      int preShift,
      const std::int32_t *values/*[4]*/,
      int shift, std::int32_t mask,
      std::int32_t *results/*[4]*/
    );

  };

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t BitExtension::RepeatSigned(
    std::int32_t value, int shift, std::int32_t mask
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_set1_epi32(value);

    return _mm_cvtsi128_si32(
      _mm_or_si128(
        input,
        _mm_and_si128(
          _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
          _mm_set1_epi32(mask)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    return value | ((value >> shift) & mask);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t BitExtension::ShiftAndRepeatSigned(
    int preShift, std::int32_t value, int shift, std::int32_t mask
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_slli_epi32(_mm_set1_epi32(value), preShift);

    return _mm_cvtsi128_si32(
      _mm_or_si128(
        input,
        _mm_and_si128(
          _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
          _mm_set1_epi32(mask)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    value <<= preShift;
    return value | ((value >> shift) & mask);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t BitExtension::TripleSigned(
    std::int32_t value, int shift, std::int32_t mask
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_set1_epi32(value);
    __m128i shifted = _mm_and_si128(
      _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
      _mm_set1_epi32(mask)
    );

    return _mm_cvtsi128_si32(
      _mm_or_si128(
        input,
        _mm_or_si128(
          shifted,
          _mm_srai_epi32(shifted, shift)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    std::int32_t shifted = (value >> shift) & mask;
    return value | shifted | (shifted >> shift);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::int32_t BitExtension::ShiftAndTripleSigned(
    int preShift, std::int32_t value, int shift, std::int32_t mask
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_slli_epi32(_mm_set1_epi32(value), preShift);
    __m128i shifted = _mm_and_si128(
      _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
      _mm_set1_epi32(mask)
    );

    return _mm_cvtsi128_si32(
      _mm_or_si128(
        input,
        _mm_or_si128(
          shifted,
          _mm_srai_epi32(shifted, shift)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    value <<= preShift;
    std::int32_t shifted = (value >> shift) & mask;
    return value | shifted | (shifted >> shift);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void BitExtension::RepeatSignedx4(
    const std::int32_t *values/*[4]*/,
    int shift, std::int32_t mask,
    std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_loadu_si128(reinterpret_cast<const __m128i *>(values));

    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_or_si128(
        input,
        _mm_and_si128(
          _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
          _mm_set1_epi32(mask)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    results[0] = values[0] | ((values[0] >> shift) & mask);
    results[1] = values[1] | ((values[1] >> shift) & mask);
    results[2] = values[2] | ((values[2] >> shift) & mask);
    results[3] = values[3] | ((values[3] >> shift) & mask);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void BitExtension::ShiftAndRepeatSignedx4(
    int preShift,
    const std::int32_t* values/*[4]*/,
    int shift, std::int32_t mask,
    std::int32_t* results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_slli_epi32(
      _mm_loadu_si128(reinterpret_cast<const __m128i*>(values)), preShift
    );

    _mm_storeu_si128(
      reinterpret_cast<__m128i*>(results),
      _mm_or_si128(
        input,
        _mm_and_si128(
          _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
          _mm_set1_epi32(mask)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    {
      std::int32_t shifted0 = values[0] << preShift;
      results[0] = shifted0 | ((shifted0 >> shift) & mask);
    }
    {
      std::int32_t shifted1 = values[1] << preShift;
      results[1] = shifted1 | ((shifted1 >> shift) & mask);
    }
    {
      std::int32_t shifted2 = values[2] << preShift;
      results[2] = shifted2 | ((shifted2 >> shift) & mask);
    }
    {
      std::int32_t shifted3 = values[3] << preShift;
      results[3] = shifted3 | ((shifted3 >> shift) & mask);
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void BitExtension::TripleSignedx4(
    const std::int32_t *values/*[4]*/,
    int shift, std::int32_t mask,
    std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_loadu_si128(reinterpret_cast<const __m128i *>(values));
    __m128i shifted = _mm_and_si128(
      _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
      _mm_set1_epi32(mask)
    );

    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_or_si128(
        input,
        _mm_or_si128(
          shifted,
          _mm_srai_epi32(shifted, shift)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    {
      std::int32_t shifted0 = (values[0] >> shift) & mask;
      results[0] = values[0] | shifted0 | (shifted0 >> shift);
    }
    {
      std::int32_t shifted1 = (values[1] >> shift) & mask;
      results[1] = values[1] | shifted1 | (shifted1 >> shift);
    }
    {
      std::int32_t shifted2 = (values[2] >> shift) & mask;
      results[2] = values[2] | shifted2 | (shifted2 >> shift);
    }
    {
      std::int32_t shifted3 = (values[3] >> shift) & mask;
      results[3] = values[3] | shifted3 | (shifted3 >> shift);
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  inline void BitExtension::ShiftAndTripleSignedx4(
    int preShift,
    const std::int32_t *values/*[4]*/,
    int shift, std::int32_t mask,
    std::int32_t *results/*[4]*/
  ) {
#if defined(NUCLEX_AUDIO_HAVE_SSE2)
    __m128i input = _mm_slli_epi32(
      _mm_loadu_si128(reinterpret_cast<const __m128i *>(values)), preShift
    );
    __m128i shifted = _mm_and_si128(
      _mm_srai_epi32(input, shift), // should use _srli_ but I want to match the C++ code.
      _mm_set1_epi32(mask)
    );

    _mm_storeu_si128(
      reinterpret_cast<__m128i *>(results),
      _mm_or_si128(
        input,
        _mm_or_si128(
          shifted,
          _mm_srai_epi32(shifted, shift)
        )
      )
    );
//#elif defined(__ARM_NEON)
#else
    {
      std::int32_t input0 = values[0] << preShift;
      std::int32_t shifted0 = (input0 >> shift) & mask;
      results[0] = input0 | shifted0 | (shifted0 >> shift);
    }
    {
      std::int32_t input1 = values[1] << preShift;
      std::int32_t shifted1 = (input1 >> shift) & mask;
      results[1] = input1 | shifted1 | (shifted1 >> shift);
    }
    {
      std::int32_t input2 = values[2] << preShift;
      std::int32_t shifted2 = (input2 >> shift) & mask;
      results[2] = input2 | shifted2 | (shifted2 >> shift);
    }
    {
      std::int32_t input3 = values[3] << preShift;
      std::int32_t shifted3 = (input3 >> shift) & mask;
      results[3] = input3 | shifted3 | (shifted3 >> shift);
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_BITEXTENSION_H
