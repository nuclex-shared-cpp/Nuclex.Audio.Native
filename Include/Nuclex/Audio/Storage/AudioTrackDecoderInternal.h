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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODERINTERNAL_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODERINTERNAL_H

#include "Nuclex/Audio/Config.h"

#include <cstdint> // for std::uint8_t, std::int16_t, std::int32_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Base class for the audio track decoder, separates the internal methods that
  ///   only implementers of custom audio track decoders need to be exposed to
  /// </summary>
  class NUCLEX_AUDIO_TYPE AudioTrackDecoderInternal {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioTrackDecoderInternal() = default;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedUint8(
      std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedInt16(
      std::int16_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedInt32(
      std::int32_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedFloat(
      float *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedDouble(
      double *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeSeparatedUint8(
      std::uint8_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeSeparatedInt16(
      std::int16_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeSeparatedInt32(
      std::int32_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeSeparatedFloat(
      float *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeSeparatedDouble(
      double *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODERINTERNAL_H
