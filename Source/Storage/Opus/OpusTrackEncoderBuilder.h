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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODERBUILDER_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODERBUILDER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/Storage/AudioTrackEncoderBuilder.h"

#include <optional> // for std::optional

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates audio track encoders for the Opus file format</summary>
  class OpusTrackEncoderBuilder : public AudioTrackEncoderBuilder {

    /// <summary>Initializes a new Opus track encoder builder</summary>
    public: OpusTrackEncoderBuilder();
    /// <summary>Frees all resources owned by the instance</summary>
    public: ~OpusTrackEncoderBuilder() override = default;

    /// <summary>Retrieves a list of supported formats for the encoded samples</summary>
    /// <returns>All supported formats in which samples can be stored</returns>
    public: const std::vector<AudioSampleFormat> &GetSupportedSampleFormats() const override;

    /// <summary>Retrieves a list of supported sample rates for the codec</summary>
    /// <returns>All supported sample rates or empty if unrestricted</returns>
    public: const std::vector<std::size_t> &GetSupportedSampleRates() const override;

    /// <summary>Retrieves a list of preferred sample rates for the codec</summary>
    /// <returns>The preferred sample rates or empty if the codec doesn't care</returns>
    public: const std::vector<std::size_t> &GetPreferredSampleRates() const override;

    /// <summary>Retrieves the channel order preferred by the encoder</summary>
    /// <param name="channels">Channels that should be put in the preferred order</param>
    /// <returns>A list of the channel in the mask in their preferred order</returns>
    public: std::vector<ChannelPlacement> GetPreferredChannelOrder(
      ChannelPlacement channels
    ) const override;

    /// <summary>Tells whether this audio codec is a lossless one</summary>
    /// <returns>True if the codec is lossless, false if it is lossy</returns>
    public: bool IsLossless() const override { return false; }

    /// <summary>Selects the format in which samples will be stored in the file</summary>
    /// <param name="format">Format to use for the encoded samples</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetSampleFormat(
      AudioSampleFormat format = AudioSampleFormat::SignedInteger_16
    ) override;

    /// <summary>Tells the encoder the sample rate of your audio data</summary>
    /// <param name="samplesPerSecond">
    ///   Sample rate in samples per second (in each channel)
    /// </param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetSampleRate(
      std::size_t samplesPerSecond = 48000
    ) override;

    /// <summary>Sets the number, placement and ordering of the input channels</summary>
    /// <param name="orderedChannels">
    ///   A list containing the channels to encode in the order you wish to feed them
    ///   to the encoder.
    /// </param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetChannels(
      const std::vector<ChannelPlacement> &orderedChannels
    ) override;

    /// <summary>Selects the bitrate which the encoder should try to match</summary>
    /// <param name="kilobitsPerSecond">Desired bit rate in kilobits per second</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetTargetBitrate(
      float kilobitsPerSecond
    ) override;

    /// <summary>Requests the amount of effort that should be used to compress</summary>
    /// <param name="effort">Effort as a value from 0.0 to 1.0</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetCompressionEffort(
      float effort = 1.0f
    ) override;

    /// <summary>Builds an audio track encoder that writes into a virtual file</summary>
    /// <param name="target">Virtual file that will receive the encoded audio data</param>
    /// <returns>The new encoder, set up to write into a file in the specified file</returns>
    public: std::shared_ptr<AudioTrackEncoder> Build(
      const std::shared_ptr<VirtualFile> &target
    ) override;

    /// <summary>The order in which the user wishes to feed us the input channels</summary>>
    private: std::vector<ChannelPlacement> inputChannelOrder;
    /// <summary>Sample rate in samples per second (i.e. 44100 or 48000)</summary>
    private: std::optional<std::size_t> sampleRate;
    /// <summary>Target bitrate for the audio file being encoded</summary>
    private: std::optional<float> targetBitrate;
    /// <summary>Amount of effort (= CPU time and/or memory) to invest</summary>
    private: float effort;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSTRACKENCODERBUILDER_H
