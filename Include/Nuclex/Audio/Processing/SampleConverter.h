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

#ifndef NUCLEX_AUDIO_PROCESSING_SAMPLECONVERTER_H
#define NUCLEX_AUDIO_PROCESSING_SAMPLECONVERTER_H

#include "Nuclex/Audio/Config.h"

#include "Nuclex/Audio/Processing/Quantization.h"
#include "Nuclex/Audio/Processing/Reconstruction.h"

#include <cstddef> // for std::size_t
#include <stdexcept> // for std::runtime_error
#include <type_traits> // for std::is_same<>
#include <vector> // for std::vector
#include <cassert> // for assert()

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts between different data tyeps used for audio samples</summary>
  /// <remarks>
  ///   <para>
  ///     This is a utility class that takes care of the operations needed to convert between
  ///     different sample data formats. All methods assume batch operations since it's rare
  ///     that could would want to only convert a single sample (but you can do that, too,
  ///     just grab a pointer to it and specify sample count as 1)
  ///   </para>
  ///   <para>
  ///     Each operation supports an arbitrary number of valid bits for integer samples,
  ///     specified in a separate parameter. This lets you deal with 12-bit audio stored
  ///     in 16-bit integers or, more commonly, 24-bit audio stored in 32-bit integers.
  ///     Any other topology is possible, too, even silly ones, such as 7-bit audio in
  ///     32-bit integers.
  ///   </para>
  ///   <para>
  ///     The occupied bits are always the most significand ones. If you have aforementioned
  ///     silly 24-bit sample in a 32-bit integer, on a big endian system, you get this:
  ///     <code>65 43 21 ..</code> (each digit meant to be one nibble, aka 4 bits).
  ///     On a little endian system, however, you get this: <code>.. 21 43 65</code>.
  ///   </para>
  ///   <para>
  ///     Integers assume symmetric quantization: while signed integers are able to go
  ///     one count deeper into the negative range as they can in the positive range
  ///     (i.e. -32768 to 32767 for a 16-bit integer), these methods assume the negative
  ///     limit is -32767, matching the positive range. Symmetric quantization is how all
  ///     common audio formats and DSPs operate.
  ///   </para>
  ///   <para>
  ///     Finally, the possible conversions are split into 4 categories: quantization and
  ///     reconstruction to convert from float to integer or integer to float respectively.
  ///     For float-to-float or integer-to-integer conversions between different sizes,
  ///     the operations are called truncate and extend and do exactly what they sound like.
  ///   </para>
  /// </remarks>
  class NUCLEX_AUDIO_TYPE SampleConverter {

    /// <summary>Converts a sample from one data format into another</summary>
    /// <typeparam name="TSourceSample">Type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TTargetSample>
    inline static void Convert(
      const TSourceSample *source, std::size_t sourceBitCount,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    );

    /// <summary>Converts floating point samples into quantized integer samples</summary>
    /// <typeparam name="TFloatSourceSample">Floating point type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Integer type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TFloatSourceSample, typename TTargetSample>
    inline static void Quantize(
      const TFloatSourceSample *source,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    );

    /// <summary>Converts quantized integer samples back into floating point samples</summary>
    /// <typeparam name="TSourceSample">Integer type of the source samples</typeparam>
    /// <typeparam name="TFloatTargetSample">Floating point type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TFloatTargetSample>
    inline static void Reconstruct(
      const TSourceSample *source, std::size_t sourceBitCount,
      TFloatTargetSample *target,
      std::size_t sampleCount
    );

    /// <summary>Truncates samples to fewer bits</summary>
    /// <typeparam name="TSourceSample">Type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TTargetSample>
    inline static void TruncateBits(
      const TSourceSample *source, std::size_t sourceBitCount,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    );

    /// <summary>Extends samples to fill more bits</summary>
    /// <typeparam name="TSourceSample">Type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TTargetSample>
    inline static void ExtendBits(
      const TSourceSample *source, std::size_t sourceBitCount,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    );

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TSourceSample, typename TTargetSample>
  inline void SampleConverter::Convert(
    const TSourceSample *source, std::size_t sourceBitCount,
    TTargetSample *target, std::size_t targetBitCount,
    std::size_t sampleCount
  ) {
    if constexpr(std::is_floating_point<TSourceSample>::value) {
      if constexpr(std::is_floating_point<TTargetSample>::value) {
        if(targetBitCount >= sourceBitCount) {
          ExtendBits(source, sourceBitCount, target, targetBitCount, sampleCount);
        } else {
          TruncateBits(source, sourceBitCount, target, targetBitCount, sampleCount);
        }
      } else {
        Quantize(source, target, targetBitCount, sampleCount);
      }
    } else if constexpr(std::is_floating_point<TTargetSample>::value) {
      Reconstruct(source, sourceBitCount, target, sampleCount);
    } else {
      if(targetBitCount >= sourceBitCount) {
        ExtendBits(source, sourceBitCount, target, targetBitCount, sampleCount);
      } else {
        TruncateBits(source, sourceBitCount, target, targetBitCount, sampleCount);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TFloatSourceSample, typename TTargetSample>
  inline void SampleConverter::Quantize(
    const TFloatSourceSample *source,
    TTargetSample *target, std::size_t targetBitCount,
    std::size_t sampleCount
  ) {
    static_assert(
      (
        std::is_same<TFloatSourceSample, float>::value ||
        std::is_same<TFloatSourceSample, double>::value
      ) && (
        std::is_same<TTargetSample, std::uint8_t>::value ||
        std::is_same<TTargetSample, std::int16_t>::value ||
        std::is_same<TTargetSample, std::int32_t>::value
      ),
      u8"This method only converts from float samples to quantized integer samples"
    );

    if constexpr(std::is_same<TTargetSample, std::uint8_t>::value) { // float -> uint8

      // Floating point to unsigned integer
      // ----------------------------------
      //
      std::int16_t midpoint = (1 << targetBitCount) / 2;
      TFloatSourceSample limit = static_cast<TFloatSourceSample>(
        (midpoint - 1) << (8 - targetBitCount)
      );
      midpoint <<= (8 - targetBitCount);
      while(3 < sampleCount) {
        std::int32_t scaled[4];
        Quantization::MultiplyToNearestInt32x4(source, limit, scaled);
        target[0] = static_cast<TTargetSample>(scaled[0] + midpoint);
        target[1] = static_cast<TTargetSample>(scaled[1] + midpoint);
        target[2] = static_cast<TTargetSample>(scaled[2] + midpoint);
        target[3] = static_cast<TTargetSample>(scaled[3] + midpoint);
        source += 4;
        target += 4;
        sampleCount -= 4;
      }
      while(0 < sampleCount) {
        target[0] = static_cast<TTargetSample>(
          Quantization::NearestInt32(source[0] * limit) + midpoint
        );
        ++source;
        ++target;
        --sampleCount;
      }

    } else { // float -> int16 and int32

      // Floating point to signed integer
      // --------------------------------
      //
      if(targetBitCount < 17) {

        // From floating point to 16-bits or less
        // --------------------------------------
        //
        TFloatSourceSample limit = static_cast<TFloatSourceSample>(
          (1 << (targetBitCount - 1)) - 1
        );
        std::size_t shift = sizeof(TTargetSample) * 8 - targetBitCount;
        while(3 < sampleCount) {
          std::int32_t scaled[4];
          Quantization::MultiplyToNearestInt32x4(source, limit, scaled);
          target[0] = static_cast<TTargetSample>(scaled[0]) << shift;
          target[1] = static_cast<TTargetSample>(scaled[1]) << shift;
          target[2] = static_cast<TTargetSample>(scaled[2]) << shift;
          target[3] = static_cast<TTargetSample>(scaled[3]) << shift;
          source += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = static_cast<TTargetSample>(
            Quantization::NearestInt32(source[0] * limit)
          ) << shift;
          ++source;
          ++target;
          --sampleCount;
        }

      // From floating point to 17-bits or more
      // --------------------------------------
      //
      } else {
        double limit = static_cast<double>((1 << (targetBitCount - 1)) - 1);
        std::size_t shift = sizeof(TTargetSample) * 8 - targetBitCount;
        while(3 < sampleCount) {
          std::int32_t scaled[4];
          Quantization::MultiplyToNearestInt32x4(source, limit, scaled);
          target[0] = static_cast<TTargetSample>(scaled[0]) << shift;
          target[1] = static_cast<TTargetSample>(scaled[1]) << shift;
          target[2] = static_cast<TTargetSample>(scaled[2]) << shift;
          target[3] = static_cast<TTargetSample>(scaled[3]) << shift;
          source += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = static_cast<TTargetSample>(
            Quantization::NearestInt32(source[0] * limit)
          ) << shift;
          ++source;
          ++target;
          --sampleCount;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSourceSample, typename TFloatTargetSample>
  inline void SampleConverter::Reconstruct(
    const TSourceSample *source, std::size_t sourceBitCount,
    TFloatTargetSample *target,
    std::size_t sampleCount
  ) {
    static_assert(
      (
        std::is_same<TSourceSample, std::uint8_t>::value ||
        std::is_same<TSourceSample, std::int16_t>::value ||
        std::is_same<TSourceSample, std::int32_t>::value
      ) && (
        std::is_same<TFloatTargetSample, float>::value ||
        std::is_same<TFloatTargetSample, double>::value
      ),
      u8"This method only converts from quantized integer samples to float samples"
    );

    // From unsigned integer to floating point
    // ---------------------------------------
    //
    if constexpr(std::is_same<TSourceSample, std::uint8_t>::value) { // uint8 -> float
      std::int16_t midpoint = (1 << sourceBitCount) / 2;
      TFloatTargetSample limit = static_cast<TFloatTargetSample>(
        (midpoint - 1) << (8 - sourceBitCount)
      );
      midpoint <<= (8 - sourceBitCount);
      for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        target[sampleIndex] = (
          static_cast<TFloatTargetSample>(
            static_cast<std::int16_t>(source[sampleIndex]) - midpoint
          ) / limit
        );
      }

    // From signed integer to floating point
    // -------------------------------------
    //
    } else { // int16 or int32 -> float

      // From 16-bits or less to floating point
      // --------------------------------------
      //
      if(sourceBitCount < 17) {
        std::size_t shift = sizeof(TSourceSample) * 8 - sourceBitCount;
        TFloatTargetSample limit = static_cast<TFloatTargetSample>(
          (1 << (sourceBitCount - 1)) - 1
        );
        while(3 < sampleCount) {
          std::int32_t shifted[4];
          shifted[0] = source[0] >> shift;
          shifted[1] = source[1] >> shift;
          shifted[2] = source[2] >> shift;
          shifted[3] = source[3] >> shift;
          Reconstruction::DivideInt32ToFloatx4(shifted, limit, target);
          source += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = Reconstruction::DivideInt32ToFloat(source[0] >> shift, limit);
          ++source;
          ++target;
          --sampleCount;
        }

      // From 17-bits or more to floating point
      // --------------------------------------
      //
      } else { // for values longer than 16 bits, we force calculations to use doubles
        std::size_t shift = sizeof(TSourceSample) * 8 - sourceBitCount;
        double limit = static_cast<double>(
          (1 << (sourceBitCount - 1)) - 1
        );
        while(3 < sampleCount) {
          std::int32_t shifted[4];
          shifted[0] = source[0] >> shift;
          shifted[1] = source[1] >> shift;
          shifted[2] = source[2] >> shift;
          shifted[3] = source[3] >> shift;
          Reconstruction::DivideInt32ToFloatx4(shifted, limit, target);
          source += 4;
          target += 4;
          sampleCount -= 4;
        }
        while(0 < sampleCount) {
          target[0] = static_cast<TFloatTargetSample>(
            Reconstruction::DivideInt32ToFloat(source[0] >> shift, limit)
          );
          ++source;
          ++target;
          --sampleCount;
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSourceSample, typename TTargetSample>
  inline void SampleConverter::TruncateBits(
    const TSourceSample *source, std::size_t sourceBitCount,
    TTargetSample *target, std::size_t targetBitCount,
    std::size_t sampleCount
  ) {
    static_assert(
      std::is_floating_point<TSourceSample>::value ==
      std::is_floating_point<TTargetSample>::value,
      u8"This method only truncates float to float or integer to integer"
    );

    // From floating point to floating point
    // -------------------------------------
    //
    if constexpr(std::is_floating_point<TSourceSample>::value) {
      bool isSameOrDoubleToFloat = (
        ((sourceBitCount == sizeof(double) * 8) && (targetBitCount == sizeof(float) * 8)) ||
        (sourceBitCount == targetBitCount)
      );
      if(!isSameOrDoubleToFloat) {
        throw std::runtime_error(
          u8"For floating point samples, truncation is only allowed from double to float"
        );
      }
      for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        target[sampleIndex] = static_cast<TTargetSample>(source[sampleIndex]);
      }
    } else { // if both types are floating point / both are integers
      static_assert(
        (
          std::is_same<TSourceSample, std::uint8_t>::value ||
          std::is_same<TSourceSample, std::int16_t>::value ||
          std::is_same<TSourceSample, std::int32_t>::value
        ) &&
        (
          std::is_same<TTargetSample, std::uint8_t>::value ||
          std::is_same<TTargetSample, std::int16_t>::value ||
          std::is_same<TTargetSample, std::int32_t>::value
        ),
        u8"This method only handles 8-bit unsigned and 16-bit/32-bit signed integers"
      );

      // Between integers with unsigned involved
      // ---------------------------------------
      //
      if constexpr(
        std::is_same<TSourceSample, std::uint8_t>::value ||
        std::is_same<TTargetSample, std::uint8_t>::value
      ) {

        // 8-bit is so rare that I haven't written custom code for it...
        assert(false && u8"Fallback code triggered, accuracy not optimal");
        std::vector<double> doubles;
        doubles.resize(sampleCount);
        Reconstruct(source, sourceBitCount, doubles.data(), sampleCount);
        Quantize(doubles.data(), target, targetBitCount, sampleCount);

      // Longer signed integer to shorter signed integer
      // -----------------------------------------------
      //
      } else { // unsigned involved / both are signed

        TTargetSample targetMask = (1 << (targetBitCount - 1)) - 1;
        targetMask |= 1 << (targetBitCount - 1);
        targetMask <<= (sizeof(TTargetSample) * 8 - targetBitCount);

        if constexpr(sizeof(TTargetSample) < sizeof(TSourceSample)) {
          std::size_t shift = (sizeof(TSourceSample) - sizeof(TTargetSample)) * 8;
          for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
            target[sampleIndex] = static_cast<TTargetSample>(
              (source[sampleIndex] >> shift) & targetMask
            );
          }
        } else { // target type shorter / longer or equal
          std::size_t shift = (sizeof(TTargetSample) - sizeof(TSourceSample)) * 8;
          for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
            target[sampleIndex] = (
              (static_cast<TTargetSample>(source[sampleIndex]) << shift) & targetMask
            );
          }
        } // if target type shorter or longer
      } // if unsigned involved or both signed
    } // if both types are float or integer
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSourceSample, typename TTargetSample>
  inline void SampleConverter::ExtendBits(
    const TSourceSample *source, std::size_t sourceBitCount,
    TTargetSample *target, std::size_t targetBitCount,
    std::size_t sampleCount
  ) {
    static_assert(
      std::is_floating_point<TSourceSample>::value ==
      std::is_floating_point<TTargetSample>::value,
      u8"This method only extends float to float or integer to integer"
    );

    if constexpr(std::is_floating_point<TSourceSample>::value) {
      bool isSameOrDoubleToFloat = (
        ((sourceBitCount == sizeof(float) * 8) && (targetBitCount == sizeof(double) * 8)) ||
        (sourceBitCount == targetBitCount)
      );
      if(!isSameOrDoubleToFloat) {
        throw std::runtime_error(
          u8"For floating point samples, extension is only allowed from float to double"
        );
      }
      for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        target[sampleIndex] = static_cast<TTargetSample>(source[sampleIndex]);
      }
    } else { // if both types are floating point / both are integers
      static_assert(
        (
          std::is_same<TSourceSample, std::uint8_t>::value ||
          std::is_same<TSourceSample, std::int16_t>::value ||
          std::is_same<TSourceSample, std::int32_t>::value
        ) &&
        (
          std::is_same<TTargetSample, std::uint8_t>::value ||
          std::is_same<TTargetSample, std::int16_t>::value ||
          std::is_same<TTargetSample, std::int32_t>::value
        ),
        u8"This method only handles 8-bit unsigned and 16-bit/32-bit signed integers"
      );

      // Between integers with unsigned involved
      // ---------------------------------------
      //
      if constexpr(
        std::is_same<TSourceSample, std::uint8_t>::value ||
        std::is_same<TTargetSample, std::uint8_t>::value
      ) {

        // 8-bit is so rare that I haven't written custom code for it...
        assert(false && u8"Fallback code triggered, accuracy not optimal");
        std::vector<double> doubles;
        doubles.resize(sampleCount);
        Reconstruct(source, sourceBitCount, doubles.data(), sampleCount);
        Quantize(doubles.data(), target, targetBitCount, sampleCount);

      // Longer signed integer to shorter signed integer
      // -----------------------------------------------
      //
      } else { // unsigned involved / both are signed

        TTargetSample targetMask = (1 << (targetBitCount - 1)) - 1;
        targetMask |= 1 << (targetBitCount - 1);
        targetMask <<= (sizeof(TTargetSample) * 8 - targetBitCount);

        if constexpr(sizeof(TTargetSample) < sizeof(TSourceSample)) {
          std::size_t shift = (sizeof(TSourceSample) - sizeof(TTargetSample)) * 8;
          if(sourceBitCount == targetBitCount) { // conversion between containing types only
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              target[sampleIndex] = static_cast<TTargetSample>(
                (source[sampleIndex] >> shift)
              );
            }
          } else { // repeat source bit pattern once

            // Converting from a longer type to a shorter type while at the same time
            // using more bits is something I don't expect to happen /at all/,
            // so I haven't written custom code for it...
            assert(false && u8"Fallback code triggered, accuracy not optimal");
            std::vector<double> doubles;
            doubles.resize(sampleCount);
            Reconstruct(source, sourceBitCount, doubles.data(), sampleCount);
            Quantize(doubles.data(), target, targetBitCount, sampleCount);          

          }
        } else { // target type shorter / longer or equal
          std::size_t shift = (sizeof(TTargetSample) - sizeof(TSourceSample)) * 8;

          if(sourceBitCount == targetBitCount) { // conversion between containing types only
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              target[sampleIndex] = (
                (static_cast<TTargetSample>(source[sampleIndex]) << shift)
              );
            }
          } else {// repeat bit pattern once or twice
            TTargetSample doubleMask = (1 << (sourceBitCount - 1)) - 1;
            if(sourceBitCount * 2 >= targetBitCount + 1) { // repeat bit pattern once
              doubleMask <<= (sizeof(TTargetSample) * 8 - sourceBitCount);
              doubleMask >>= (sourceBitCount - 1);
              for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
                TTargetSample value = static_cast<TTargetSample>(source[sampleIndex]) << shift;
                TTargetSample once = (value >> (sourceBitCount - 1)) & doubleMask;
                target[sampleIndex] = targetMask & (value | once);
              }
            } else { // repeat bit pattern once / twice
              doubleMask <<= (sizeof(TTargetSample) * 8 - sourceBitCount - (sourceBitCount - 1));
              //TTargetSample tripleMask = doubleMask >> (sourceBitCount - 1);
              for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
                TTargetSample value = static_cast<TTargetSample>(source[sampleIndex]) << shift;
                TTargetSample once = (value >> (sourceBitCount - 1)) & doubleMask;
                TTargetSample twice = (once >> (sourceBitCount - 1));
                target[sampleIndex] = targetMask & (value | once | twice);
              }
            }

          }

        } // if target type shorter or longer

      } // if unsigned involved or both signed
    } // if both types are float or integer
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Processing

#endif // NUCLEX_AUDIO_PROCESSING_SAMPLECONVERTER_H
