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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/AudioSampleFormat.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <memory> // for std::unique_ptr, std::shared_ptr
#include <cstdint> // for std::uint64_t
#include <vector> // for std::vector

#include <wavpack.h> // for the WavPack types

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  struct TrackInfo;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  struct ReadOnlyStreamAdapterState;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility class with intermediate methods used to decode WavPack files</summary>
  /// <remarks>
  ///   <para>
  ///     This wrapper handles most of the interfacing with libwavpack, allowing other
  ///     classes that are part of the WavPack codec implementation to focus more on
  ///     the actual business logic.
  ///   </para>
  ///   <para>
  ///     Because libwavpack uses file cursors in its design, this class is also designed to
  ///     be used from a single thread and requires mutexes to sequentialize access to it.
  ///   </para>
  /// </remarks>
  class WavPackReader {

    /// <summary>Determines the native sample format from WavPack's parameters</summary>
    /// <param name="mode">Encoder mode that WavPack file was encoded with</param>
    /// <param name="bitsPerSample">The number of valid bits in each sample</param>
    /// <returns>The equivalent sample format enumeration value</returns>
    public: static AudioSampleFormat SampleFormatFromModeAndBitsPerSample(
      int mode, int bitsPerSample
    );

    /// <summary>Initializes a new WavPack reader accessing the specified file</summary>
    /// <param name="file">File that will be opened by the reader</param>
    public: WavPackReader(const std::shared_ptr<const VirtualFile> &file);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~WavPackReader();

    /// <summary>Reads the WavPack file's metadata</summary>
    /// <param name="target">Track information container that will receive the metadata</param>
    public: void ReadMetadata(TrackInfo &target);

    /// <summary>Counts the total number of frames (= samples in each channel)</summary>
    /// <returns>The total number of frames in the audio file</returns>
    public: std::uint64_t CountTotalFrames() const;

    /// <summary>Determines the closest sample format to the file's audio data</summary>
    /// <returns>A sample format matching or able to store the file's audio data</returns>
    public: AudioSampleFormat GetSampleFormat() const;

    /// <summary>Gets the order in which interlaved samples are decoded</summary>
    /// <returns>A list of channels in the order they are interleaved</returns>
    public: std::vector<ChannelPlacement> GetChannelOrder() const;

    /// <summary>Tries to figure out the size of the current block</summary>
    /// <returns>The size of the current block</returns>
    public: std::size_t GetCurrentBlockSize() const;

    /// <summary>Retrieves the current position of the frame cursor</summary>
    /// <returns>The frame cursor, pointing at the frame that will be decoded next</returns>
    public: std::uint64_t GetFrameCursorPosition() const;

    /// <summary>Moves the frame cursor to the specified location</summary>
    /// <param name="frameIndex">Index of the frame that should be decoded next</param>
    public: void Seek(std::uint64_t frameIndex);

    /// <summary>Decodes samples from the audio file in interleaved format</summary>
    /// <typename name="TSample">Type of samples that will be decoded</typename>
    /// <param name="target">Buffer into which the samples will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    public: template<typename TSample>
    void DecodeInterleaved(TSample *target, std::size_t frameCount);

    /// <summary>Decodes samples from the audio file in interleaved format</summary>
    /// <param name="targets">Buffers into which the channels will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    public: template<typename TSample>
    void DecodeSeparated(TSample *targets[], std::size_t frameCount);

    /// <summary>Decodes samples from the audio file and converts them</summary>
    /// <typename name="TSample">Type of samples to convert to</typename>
    /// <typename name="BitsPerSampleOver16">
    ///   Whether the *decoded* bits per sample is above 16 (whether the output
    ///   type has more than 16 bits is trivial to determine via sizeof())
    /// </typename>
    /// <typename name="WidenFactor">
    ///   How many times the *decoded* bits have to be repeated to fill the output
    ///   data type. Set to 0 (zero) if the *decoded* data type is float.
    /// <typename>
    /// <param name="target">Buffer into which the samples will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    /// <remarks>
    ///   This method is invoked if a target format other than float is requested,
    ///   performing an SSE2 SIMD-enhanced conversion of the samples to the requested
    ///   target type at the decoded block level.
    /// </remarks>
    private: template<
      typename TSample, // output type
      bool BitsPerSampleOver16 = false, // for decoded bits per sample, not TSample
      std::size_t WidenFactor = 1 // set to 0 if decoded samples are float!
    >
    void decodeInterleavedAndConvert(TSample *target, std::size_t frameCount);

    /// <summary>Decodes samples from the audio file and converts them</summary>
    /// <typename name="TSample">Type of samples to convert to</typename>
    /// <param name="targets">Buffers into which the channels will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    /// <remarks>
    ///   This method is invoked if the channels need to be separated. Since libopusfile
    ///   always delivers samples in interleaved format, each sample needs an additional
    ///   copy this way. If a sample format other than float is the target, it will also
    ///   performing an SSE2 SIMD-enhanced conversion at the decoded block level.
    /// </remarks>
    private: template<typename TSample>
    void decodeInterleavedConvertAndSeparate(TSample *targets[], std::size_t frameCount);

    /// <summary>File the reader is accessing</summary>
    private: std::shared_ptr<const VirtualFile> file;
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

    /// <summary>Encoding mode of the WavPack file (for floating point check)</summary>
    private: int mode;
    /// <summary>Number of bits per sample</summary>
    private: int bitsPerSample;
    /// <summary>Number of bytes per sample</summary>
    private: int bytesPerSample;
    /// <summary>Number of channels in the audio file</summary>
    private: std::size_t channelCount;
    /// <summary>Number of samples per second (per channel)</summary>
    private: std::size_t sampleRate;
    /// <summary>Index of the frame that will be decoded next</summary>
    private: std::uint64_t frameCursor;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H
