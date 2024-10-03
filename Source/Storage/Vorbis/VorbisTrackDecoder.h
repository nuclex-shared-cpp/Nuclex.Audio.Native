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

#ifndef NUCLEX_AUDIO_STORAGE_VORBIS_VORBISTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_VORBIS_VORBISTRACKDECODER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "Nuclex/Audio/Storage/AudioTrackDecoder.h"
#include "Nuclex/Audio/TrackInfo.h"
#include "./VorbisReader.h"

#include <mutex> // for std::mutex

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes Vorbis audio tracks</summary>
  class VorbisTrackDecoder : public AudioTrackDecoder {

    /// <summary>Initializes a new Vorbis track decoder on the specified file</summary>
    /// <param name="file">File that will be opened and decoded</param>
    public: VorbisTrackDecoder(const std::shared_ptr<const VirtualFile> &file);
    /// <summary>Frees all resources owned by the instance</summary>
    public: ~VorbisTrackDecoder() override;

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

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeSeparatedUint8(
      std::uint8_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeSeparatedInt16(
      std::int16_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeSeparatedInt32(
      std::int32_t *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeSeparatedFloat(
      float *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Decodes audio channels, separated, into the target buffers</summary>
    /// <typeparam name="TSample">Type of samples to decode as</typeparam>
    /// <param name="buffers">Buffers in which the channels will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeSeparatedDouble(
      double *buffers[], const std::uint64_t startFrame, const std::size_t frameCount
    ) const override;

    /// <summary>Throws an exception if the decoding range is out of bounds</summary>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    private: void verifyDecodeRange(
      const std::uint64_t startFrame, const std::size_t frameCount
    ) const;

    /// <summary>Reader through which the audio file will be decoded</summary>
    private: mutable VorbisReader reader;
    /// <summary>Informations about the audio track being decoded</summary>
    private: TrackInfo trackInfo;
    /// <summary>Order in which audio channels appear</summary>
    private: std::vector<ChannelPlacement> channelOrder;
    /// <summary>Total number of frames in the Vorbis stream</summary>
    /// <remarks>
    ///   This can be unknown if the Ogg Vorbis file is being cut off at the beginning,
    ///   but this library is designed for usage with plain and complete files,
    ///   at most wrapped in a media container or archive.
    /// </remarks>
    private: std::uint64_t totalFrameCount;
    /// <summary>Must be held while decoding</summary>
    private: mutable std::mutex decodingMutex;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)

#endif // NUCLEX_AUDIO_STORAGE_VORBIS_VORBISTRACKDECODER_H
