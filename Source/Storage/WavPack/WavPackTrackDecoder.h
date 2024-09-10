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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/Storage/AudioTrackDecoder.h"
#include "./WavPackVirtualFileAdapter.h"
#include "../../Platform/WavPackApi.h"

#include <mutex> // for std::mutex

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes WavPack audio tracks</summary>
  class WavPackTrackDecoder : public AudioTrackDecoder {

    /// <summary>Determines the native sample format from WavPack's parameters</summary>
    /// <param name="mode">Encoder mode that WavPack file was encoded with</param>
    /// <param name="bitsPerSample">The number of valid bits in each sample</param>
    /// <returns>The equivalent sample format enumeration value</returns>
    public: static AudioSampleFormat SampleFormatFromModeAndBitsPerSample(
      int mode, int bitsPerSample
    );

    /// <summary>Initializes a new WavPack track decoder on the specified file</summary>
    /// <param name="file">File that will be opened and decoded</param>
    public: WavPackTrackDecoder(const std::shared_ptr<const VirtualFile> &file);
    /// <summary>Frees all resources owned by the instance</summary>
    public: ~WavPackTrackDecoder() override = default;

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
      std::int16_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedInt32(
      std::int32_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedFloat(
      float *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const override;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: void DecodeInterleavedDouble(
      double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const override;

    /// <summary>Fetches the order of audio channels from the WavPack context</summary>
    private: void fetchChannelOrder();

    /// <summary>
    ///   Stores callbacks through which libwavpack accesses the VirtualFile instance
    /// </summary>
    /// <remarks>
    ///   When opening the WavPack file, a pointer to this structure is stored Within
    ///   the WavPackContext, so it needs to continue to exist.
    /// </remarks>
    private: ::WavpackStreamReader64 streamReader;
    /// <summary>Holds the state of the VirtualFile stream adapter</summary>
    /// <remarks>
    ///   This stores the emulated file cursor for the specific WavPack context and 
    ///   records any exceptions from the VirtualFile so they do not have to pass through
    ///   libwavpack, which is C code and won't anticipate being interrupted this way.
    /// </remarks>
    private: std::unique_ptr<ReadOnlyStreamAdapterState> state;
    /// <summary>Represents the opened WavPack file for the libwavpack API</summary>
    /// <remarks>
    ///   This ties all the internal WavPack data structure together and is passed
    ///   to any public API method in libwavpack we can call.
    /// </remarks>
    private: std::shared_ptr<::WavpackContext> context;
    /// <summary>Order in which audio channels appear</summary>
    private: std::vector<ChannelPlacement> channelOrder;
    /// <summary>Total number of samples in the WavPack file</summary>
    /// <remarks>
    ///   This can be unknown if the WavPack file is being streamed, was truncated and
    ///   perhaps some other cases, but this library is designed for usage with plain
    ///   and complete WavPack files, at most wrapped in a media container or archive.
    /// </remarks>
    private: std::uint64_t totalSampleCount;
    /// <summary>Format in which the samples are deliverd by libwavpack</summary>
    private: AudioSampleFormat sampleFormat;
    /// <summary>Number of valid bits in the audio sample from the WavPack file</summary>
    private: std::size_t bitsPerSample;
    /// <summary>Known position of libwavpacks cursor within the audio data</summary>
    private: mutable std::uint64_t sampleCursor;
    /// <summary>Must be held while decoding</summary>
    private: mutable std::mutex decodingMutex;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H
