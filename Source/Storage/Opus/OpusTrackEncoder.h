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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODER_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/Storage/AudioTrackEncoder.h"

#include "./OpusVirtualFileAdapter.h"

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Encodes audio channels to an Opus audio stream</summary>
  class OpusTrackEncoder : public AudioTrackEncoder {

    /// <summary>Initializes a new Opus audio track encoder</summary>
    /// <param name="target">File into which the encoded audio stream will be written</param>
    /// <param name="inputChannelOrder">Order in which the channels will be fed in</param>
    /// <param name="sampleRate">Intended playback rate in samples per second</param>
    public: OpusTrackEncoder(
      const std::shared_ptr<VirtualFile> &target,
      const std::vector<ChannelPlacement> &inputChannelOrder,
      std::size_t sampleRate
    );

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~OpusTrackEncoder() override;

    /// <summary>Retrieves the channel order the encoder expects its samples in</summary>
    /// <returns>A list of channels in the order the encoder expects them</returns>
    /// <remarks>
    ///   This order is configured in the encode builder and applied to both the interleaved
    ///   ingestion methods and the separated ingestion methods.
    /// </remarks>
    public: const std::vector<ChannelPlacement> &GetChannelOrder() const override;

    /// <summary>Selects the target bitrate for the Opus stream</summary>
    /// <param name="kiloBitsPerSecond">Target bitrate in kilobits per second</param>
    /// <remarks>
    ///   This method should only be called before the first chunk of audio is encoded.
    ///   The builder/encoder interface split enforces this for the user of the library,
    ///   but to keep the implementation from fragmenting too much, these methods are
    ///   part of this class internally.
    /// </remarks>
    public: void SetBitrate(float kilobitsPerSecond);

    /// <summary>Selects the effort the encoder should invest for compression</summary>
    /// <param name="effort">Amount of effort on a scale from 0.0 to 1.0</param>
    /// <remarks>
    ///   This method should only be called before the first chunk of audio is encoded.
    ///   The builder/encoder interface split enforces this for the user of the library,
    ///   but to keep the implementation from fragmenting too much, these methods are
    ///   part of this class internally.
    /// </remarks>
    public: void SetEffort(float effort);

    /// <summary>Flushes any audio samples remaining in the buffer</summary>
    public: void Flush() override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeInterleavedUint8(
      const std::uint8_t *buffer, std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeInterleavedInt16(
      const std::int16_t *buffer, std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeInterleavedInt32(
      const std::int32_t *buffer, std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeInterleavedFloat(
      const float *buffer, std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeInterleavedDouble(
      const double *buffer, std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeSeparatedUint8(
      const std::uint8_t *buffers[], std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeSeparatedInt16(
      const std::int16_t *buffers[], std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeSeparatedInt32(
      const std::int32_t *buffers[], std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeSeparatedFloat(
      const float *buffers[], std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    protected: void EncodeSeparatedDouble(
      const double *buffers[], std::size_t frameCount
    ) override;

    /// <summary>Encodes audio frames, interleaved, into the virtual file</summary>
    /// <typeparam name="TSample">Type of samples that will be encoded</typeparam>
    /// <param name="buffer">Buffer holding the samples that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    template<typename TSample>
    void encodeInterleaved(const TSample *buffer, std::size_t frameCount);

    /// <summary>Encodes audio frames, separated, into the virtual file</summary>
    /// <typeparam name="TSample">Type of samples that will be encoded</typeparam>
    /// <param name="buffers">Buffer holding the channels that will be encoded</param>
    /// <param name="frameCount">Number of audio frames that will be encoded</param>
    template<typename TSample>
    void encodeSeparated(const TSample *buffers[], std::size_t frameCount);

    /// <summary>Order in which the channels will be fed to the encoder</summary>
    private: std::vector<ChannelPlacement> inputChannelOrder;
    /// <summary>Whether the channel order matches the Vorbis ordering</summary>
    /// <remarks>
    ///   If this is true, interleaved floating point sampels can be fed as-is.
    /// </remarks>
    private: bool isVorbisChannelOrder;
    /// <summary>Holds the function pointers to the file I/O functions</summary>
    /// <remarks>
    ///   libopusenc takes a pointer to these, so we try to err on the side of caution
    ///   and keep the structure around for as long as the opus encoder exists.
    /// </remarks>
    private: ::OpusEncCallbacks encoderCallbacks;
    /// <summary>State (emulated file cursor, errors) of the virtual file adapter</summary>
    private: std::unique_ptr<WritableFileAdapterState> state;
    /// <summary>Comment record that will be written to the Opus stream</summary>
    private: std::shared_ptr<::OggOpusComments> opusComments;
    /// <summary>Encoder that turns raw audio data into an Opus stream</summary>
    private: std::shared_ptr<::OggOpusEnc> opusEncoder;

  };

  // ------------------------------------------------------------------------------------------- //


}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODER_H
