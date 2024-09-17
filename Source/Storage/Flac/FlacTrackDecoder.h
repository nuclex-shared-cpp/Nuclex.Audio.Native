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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACTRACKDECODER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/Storage/AudioTrackDecoder.h"

#include <mutex> // for std::mutex

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  class FlacReader;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes Flac audio tracks</summary>
  class FlacTrackDecoder : public AudioTrackDecoder {

    /// <summary>Initializes a new Flac track decoder on the specified file</summary>
    /// <param name="file">File that will be opened and decoded</param>
    public: FlacTrackDecoder(const std::shared_ptr<const VirtualFile> &file);
    /// <summary>Frees all resources owned by the instance</summary>
    public: ~FlacTrackDecoder() override;

    /// <summary>Creates a clone of the audio track decoder</summary>
    /// <returns>A clone of the audio track decoder that can be used independently</returns>
    public: std::shared_ptr<AudioTrackDecoder> Clone() const override;

    /// <summary>Counts the number of audio channels in the track</summary>
    /// <returns>The number of audio channels in the audio track</returns>
    public: std::size_t CountChannels() const override;

    /// <summary>Retrieves the order in which channels are interleaved</summary>
    /// <returns>A list of single channels in the order they're interleaved</returns>
    public: const std::vector<ChannelPlacement> &GetChannelOrder() const override;

    /// <summary>Returns the number of frames (sample count in any one channel)</summary>
    /// <returns>The number of frames the audio file is long</returns>
    public: std::uint64_t CountFrames() const override;

    /// <summary>Returns the format in which samples are obtained from the codec</summary>
    /// <returns>The format in which the audio samples are delivered by the codec</returns>
    public: AudioSampleFormat GetNativeSampleFormat() const override;

    /// <summary>Whether the audio codec directly decodes to interleaved channels</summary>
    /// <returns>True if the codec decodes straight to interleaved channels</summary>
    public: bool IsNativelyInterleaved() const override;

    // The public variant of the DecoeInterleaved() method delegates to our implementations
    public: using AudioTrackDecoder::DecodeInterleaved;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedUint8(
      std::uint8_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedInt16(
      std::int16_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedInt32(
      std::int32_t *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedFloat(
      float *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedDouble(
      double *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Fetches the order of audio channels from the Flac context</summary>
    private: void fetchChannelOrder();

    /// <summary>Reader through which the audio file will be decoded</summary>
    private: std::unique_ptr<FlacReader> reader;
    /// <summary>Order in which audio channels appear</summary>
    private: std::vector<ChannelPlacement> channelOrder;
    /// <summary>Total number of samples in the Flac file</summary>
    /// <remarks>
    ///   This can be unknown if the Flac file is being streamed, was truncated and
    ///   perhaps some other cases, but this library is designed for usage with plain
    ///   and complete Flac files, at most wrapped in a media container or archive.
    /// </remarks>
    private: std::uint64_t totalSampleCount;
    /// <summary>Format in which the samples are deliverd by libwavpack</summary>
    private: AudioSampleFormat sampleFormat;
    /// <summary>Number of valid bits in the audio sample from the Flac file</summary>
    private: std::size_t bitsPerSample;
    /// <summary>Known position of the stream decoder's cursor in the audio data</summary>
    private: mutable std::uint64_t sampleCursor;
    /// <summary>Must be held while decoding</summary>
    private: mutable std::mutex decodingMutex;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACTRACKDECODER_H
