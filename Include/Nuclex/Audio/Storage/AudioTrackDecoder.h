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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"
#include "Nuclex/Audio/AudioSampleFormat.h"

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
  class NUCLEX_AUDIO_TYPE AudioTrackDecoder {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioTrackDecoder() = default;

    /// <summary>Creates a clone of the audio track decoder</summary>
    /// <returns>A clone of the audio track decoder that can be used independently</returns>
    /// <remarks>
    ///   <para>
    ///     The underlying encoders of the audio libraries already have to handle access
    ///     from multiple threads, but unless the respective audio library explicitly states
    ///     that simultaneous requests from multiple threads are okay and uses a cursorless
    ///     design (or supports user-created decoder instances), any audio decoding will
    ///     happen sequentially.
    ///   </para>
    ///   <para>
    ///     Using this method, the audio file will be &quot;opened&quot; a second time
    ///     (actually the same <see cref="VirtualFile" /> instance is shared, which is
    ///     trivially doable because <see cref="VirtualFile" /> is cursorless.
    ///   </para>
    /// </remarks>
    public: virtual std::shared_ptr<AudioTrackDecoder> Clone() const = 0;

    /// <summary>Counts the number of audio channels in the track</summary>
    /// <returns>The number of audio channels in the audio track</returns>
    public: virtual std::size_t CountChannels() const = 0;

    /// <summary>Retrieves the order in which channels are interleaved</summary>
    /// <returns>A list of single channels in the order they're interleaved</returns>
    /// <remarks>
    ///   <para>
    ///     Audio data is generally encoded in an interleaved format, meaning that instead of
    ///     storing the whole of each channel one after another, one audio sample of each
    ///     channel is stored round-robin.
    ///   </para>
    ///   <para>
    ///     That means that for a 3 channel audio file (let's assume left, center, right),
    ///     the audio data would store sample 1: left, center, right, then
    ///     sample 2: left, center, right, then sample 3 and so on.
    ///   </para>
    ///   <para>
    ///     This is needed so that there's no seeking involved with playback, but it also
    ///     means that you need to pay attention to the ordering of channels in the decoded
    ///     audio data if you want to do anything useful with them. This method gives you
    ///     the audio channels in the order in which they are interleaved in the audio data.
    ///   </para>
    /// </remarks>
    public: virtual const std::vector<ChannelPlacement> &GetChannelOrder() const = 0;

    /// <summary>Returns the number of frames (sample count in any one channel)</summary>
    /// <returns>The number of frames the audio file is long</returns>
    public: virtual std::uint64_t CountFrames() const = 0;

    /// <summary>Returns the format in which samples are obtained from the codec</summary>
    /// <returns>The format in which the audio samples are delivered by the codec</returns>
    /// <remarks>
    ///   Keeping things in the native format for the file usually only makes sense if you
    ///   want to losslessly transcode it. Lossy formats usually use complex transformations
    ///   that work on floating point data (or emulate floating point operations, such as
    ///   when building libopus in integer-only mode), so if you transcode to a lossy format
    ///   or run filters, the only formats that make sense are float and double, the first
    ///   thing the lossy encoder will do is convert your samples to float anyway.
    /// </remarks>
    public: virtual AudioSampleFormat GetNativeSampleFormat() const = 0;

    /// <summary>Whether the audio codec directly decodes to interleaved channels</summary>
    /// <returns>True if the codec decodes straight to interleaved channels</summary>
    /// <remarks>
    ///   <para>
    ///     For better compression, codecs may separate the audio channels (consider
    ///     joint stereo, where there's a combined channel and a channel containing only
    ///     the different between right and left speaker), meaning that
    ///     <see cref="DecodeSeparated" /> will be faster. FLAC is such a codec.
    ///   </para>
    ///   <para>
    ///     Other codecs instead either store their audio channels interleaved (this is
    ///     the case for Microsoft Waveform audio files) or decided to only expose their
    ///     decoded audio data in an interleaved format to reduce confusion. In such cases,
    ///     using <see cref="DecodeInterleaved" /> will be faster.
    ///   </para>
    ///   <para>
    ///     This method can tell you which of the two decoding methods will perform better
    ///     for a given audio file. Use it if you're hunting for performance or ignore it
    ///     and just choose the most convenient format for your use case, Nuclex.Audio.Native
    ///     is very streamlined when it comes to sample format and topology conversions.
    ///   </para>
    /// </remarks>
    public: virtual bool IsNativelyInterleaved() const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <typeparam name="TSample">Type of samples to decode into</typeparam>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    /// <remarks>
    ///   <para>
    ///     The term 'frame' refers to a set of one sample for each channel. So if you decode
    ///     one frame of a 5.1 audio file, you will get 6 samples. The buffer needs to have
    ///     enough space to fit the number of frames, times the number of channels, times
    ///     the size of each sample:
    ///   </para>
    ///   <para>
    ///     <code>bufferSize = frameCount x channelCount x sizeof(TSample)</code>
    ///   </para>
    ///   <para>
    ///     It is more efficient to start reading at the closest block and to achieve best
    ///     performance, decode the file stricly sequentially, by requesting consecutive
    ///     sample ranges starting and ending at block boundaries.
    ///   </para>
    /// </remarks>
    public: template<typename TSample>
    NUCLEX_AUDIO_API void DecodeInterleaved(
      TSample *buffer, const std::uint64_t startFrame, const std::size_t frameCount
    ) const;

#if defined(PLANNED_FEATURES)

    // GetBlockSize(std::size_t sampleIndex)
    // GetBlockStart(std::size_t sampleIndex)
    //
    //   This is probably the interface that makes the most sense,

    // GetBlockSize()
    //
    //   Would return the size of the codec's blocks (all codecs are splitting audio into
    //   individually decodable packets because otherwise, playback with seking would be
    //   impossible).
    //
    //   This could be merely informative (and requesting a "page in" or however it's called
    //   would round outwards to the next block boundaries).
    //
    //   Waveform would have a block size of 1 sample.

#endif // defined(PLANNED_FEATURES)

    //
    // *** public interface ends here, all methods below are protected or private ***
    //

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
      std::int16_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedInt32(
      std::int32_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedFloat(
      float *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const = 0;

    /// <summary>Decodes audio frames, interleaved, into the target buffer</summary>
    /// <param name="buffer">Buffer in which the interleaved samples will be stored</param>
    /// <param name="startFrame">Index of the first frame to decode</param>
    /// <param name="frameCount">Number of audio frames that will be decoded</param>
    protected: virtual void DecodeInterleavedDouble(
      double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
    ) const = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  NUCLEX_AUDIO_API inline void AudioTrackDecoder::DecodeInterleaved(
    TSample *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
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
  inline void AudioTrackDecoder::DecodeInterleaved(
    std::uint8_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    DecodeInterleavedUint8(buffer, startSample, sampleCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackDecoder::DecodeInterleaved(
    std::int16_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    DecodeInterleavedInt16(buffer, startSample, sampleCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackDecoder::DecodeInterleaved(
    std::int32_t *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    DecodeInterleavedInt32(buffer, startSample, sampleCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackDecoder::DecodeInterleaved(
    float *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    DecodeInterleavedFloat(buffer, startSample, sampleCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void AudioTrackDecoder::DecodeInterleaved(
    double *buffer, const std::uint64_t startSample, const std::size_t sampleCount
  ) const {
    DecodeInterleavedDouble(buffer, startSample, sampleCount);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H
