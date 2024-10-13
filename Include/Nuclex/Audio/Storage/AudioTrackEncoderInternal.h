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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERINTERNAL_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERINTERNAL_H

#include "Nuclex/Audio/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t, std::int16_t, std::int32_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Base class for the audio track encoder, separates the internal methods that
  ///   only implementers of custom audio track encoders need to be exposed to
  /// </summary>
  class NUCLEX_AUDIO_TYPE AudioTrackEncoderInternal {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioTrackEncoderInternal() = default;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeInterleavedUint8(
      const std::uint8_t *buffer, std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeInterleavedInt16(
      const std::int16_t *buffer, std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeInterleavedInt32(
      const std::int32_t *buffer, std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeInterleavedFloat(
      const float *buffer, std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeInterleavedDouble(
      const double *buffer, std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeSeparatedUint8(
      const std::uint8_t *buffers[], std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeSeparatedInt16(
      const std::int16_t *buffers[], std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeSeparatedInt32(
      const std::int32_t *buffers[], std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeSeparatedFloat(
      const float *buffers[], std::size_t frameCount
    ) = 0;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: virtual void EncodeSeparatedDouble(
      const double *buffers[], std::size_t frameCount
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERINTERNAL_H
