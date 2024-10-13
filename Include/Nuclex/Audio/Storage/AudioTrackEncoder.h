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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"
#include "Nuclex/Audio/AudioSampleFormat.h"

#include "Nuclex/Audio/Storage/AudioTrackEncoderInternal.h"

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr
#include <cstdint> // for std::uint8_t, std::int16_t, std::int32_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes audio of all channels in one audio track</summary>
  class NUCLEX_AUDIO_TYPE AudioTrackEncoder : public AudioTrackEncoderInternal {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioTrackEncoder() = default;

    /// <summary>Retrieves the channel order the encoder expects its samples in</summary>
    /// <returns>A list of channels in the order the encoder expects them</returns>
    /// <remarks>
    ///   This order is configured in the encode builder and applied to both the interleaved
    ///   ingestion methods and the separated ingestion methods.
    /// </remarks>
    public: NUCLEX_AUDIO_API
    virtual const std::vector<ChannelPlacement> &GetChannelOrder() const = 0;

    /// <summary>Encodes a series of audio samples from an interleaved buffer</summary>
    /// <typeparam name="TSample">Type as which the samples will be fed</typeparam>
    /// <param name="buffer">Buffer in which the samples to encode are stored</param>
    /// <param name="frameCount">Number of frames (samples per channel) to encode</param>
    public: template<typename TSample>
    inline void EncodeInterleaved(const TSample *buffer, std::size_t frameCount);

    /// <summary>Encodes a series of channels provided as separate buffers</summary>
    /// <typeparam name="TSample">Type as which the samples will be fed</typeparam>
    /// <param name="buffers">Buffers storing the channels to encode</param>
    /// <param name="frameCount">Number of frames (samples per channel) to encode</param>
    public: template<typename TSample>
    inline void EncodeSeparated(const TSample *buffers[], std::size_t frameCount);

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const TSample *buffer, std::size_t frameCount
  ) {
    static_assert(
      std::is_same<TSample, std::uint8_t>::value ||
      std::is_same<TSample, std::int16_t>::value ||
      std::is_same<TSample, std::int32_t>::value ||
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value,
      u8"Only 8 but unsigned, 16/32 bit signed, float or double samples are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const std::uint8_t *buffer, std::size_t frameCount
  ) {
    EncodeInterleavedUint8(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const std::int16_t *buffer, std::size_t frameCount
  ) {
    EncodeInterleavedInt16(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const std::int32_t *buffer, std::size_t frameCount
  ) {
    EncodeInterleavedInt32(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const float *buffer, std::size_t frameCount
  ) {
    EncodeInterleavedFloat(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeInterleaved(
    const double *buffer, std::size_t frameCount
  ) {
    EncodeInterleavedDouble(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  inline void AudioTrackEncoder::EncodeSeparated(
    const TSample *buffers[], std::size_t frameCount
  ) {
    static_assert(
      std::is_same<TSample, std::uint8_t>::value ||
      std::is_same<TSample, std::int16_t>::value ||
      std::is_same<TSample, std::int32_t>::value ||
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value,
      u8"Only 8 but unsigned, 16/32 bit signed, float or double samples are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeSeparated(
    const std::uint8_t *buffers[], std::size_t frameCount
  ) {
    EncodeSeparatedUint8(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeSeparated(
    const std::int16_t *buffers[], std::size_t frameCount
  ) {
    EncodeSeparatedInt16(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeSeparated(
    const std::int32_t *buffers[], std::size_t frameCount
  ) {
    EncodeSeparatedInt32(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeSeparated(
    const float *buffers[], std::size_t frameCount
  ) {
    EncodeSeparatedFloat(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackEncoder::EncodeSeparated(
    const double *buffers[], std::size_t frameCount
  ) {
    EncodeSeparatedDouble(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODER_H
