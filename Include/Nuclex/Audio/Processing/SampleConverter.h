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

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

// Does adding a 'stride' parameter make sense here?
//
// I think normally, you'd want to always convert all samples, but perhaps there are
// edge cases (i.e. you're very sure you only need one of two channels) where this
// might make sense?
//
// Left out for now to keep the interface simple, easy to add if needed.
//

namespace Nuclex { namespace Audio { namespace Processing {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts between different data tyeps used for audio samples</summary>
  class NUCLEX_AUDIO_TYPE SampleConverter {

    /// <summary>Converts floating point samples into quantized integer samples</summary>
    /// <typeparam name="TFloatSourceSample">Floating point type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Integer type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TFloatSourceSample, typename TTargetSample>
    static void Quantize(
      const TFloatSourceSample *source,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    ) { throw std::runtime_error(u8"Not implemented yet"); }

    /// <summary>Converts quantized integer samples back into floating point samples</summary>
    /// <typeparam name="TSourceSample">Integer type of the source samples</typeparam>
    /// <typeparam name="TFloatTargetSample">Floating point type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TFloatTargetSample>
    static void Reconstruct(
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
    static void TruncateBits(
      const TSourceSample *source, std::size_t sourceBitCount,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    ) { throw std::runtime_error(u8"Not implemented yet"); }

    /// <summary>Extends samples to fill more bits</summary>
    /// <typeparam name="TSourceSample">Type of the source samples</typeparam>
    /// <typeparam name="TTargetSample">Type of the target samples</typeparam>
    /// <param name="source">Pointer to the first source sample</param>
    /// <param name="sourceBitCount">Number of valid bits in the source samples</param>
    /// <param name="target">Pointer to which the converted samples will be written</param>
    /// <param name="targetBitCount">Number of valid bits in the target samples</param>
    /// <param name="sampleCount">Number of samples that will be converted</param>
    public: template<typename TSourceSample, typename TTargetSample>
    static void ExtendBits(
      const TSourceSample *source, std::size_t sourceBitCount,
      TTargetSample *target, std::size_t targetBitCount,
      std::size_t sampleCount
    ) { throw std::runtime_error(u8"Not implemented yet"); }

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TSourceSample, typename TFloatTargetSample>
  void SampleConverter::Reconstruct(
    const TSourceSample *source, std::size_t sourceBitCount,
    TFloatTargetSample *target,
    std::size_t sampleCount
  ) {
    static_assert(
      (
        std::is_same<TSourceSample, std::uint8_t>::value ||
        std::is_same<TSourceSample, std::int16_t>::value ||
        std::is_same<TSourceSample, std::int32_t>::value ||
        std::is_same<TFloatTargetSample, float>::value ||
        std::is_same<TFloatTargetSample, double>::value
      ) &&
      u8"This method only converts from quantized integer samples to float samples"
    );

    // 8-bit unsigned int to float
    if constexpr(std::is_same<TSourceSample, std::uint8_t>::value) {
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
    } else {
      if(sourceBitCount < 17) {
        TFloatTargetSample limit = static_cast<TFloatTargetSample>(
          ((1 << (sourceBitCount - 1)) - 1) 
          
          << (sizeof(TSourceSample) * 8 - sourceBitCount)
        );
        for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
          target[sampleIndex] = (
            static_cast<TFloatTargetSample>(source[sampleIndex]) / limit
          );
        }
      } else {
        throw std::runtime_error(u8"Unsigned reconstruction not implemented yet");
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOCODEC_H
