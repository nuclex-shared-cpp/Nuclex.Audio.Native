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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/ChannelPlacement.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint64_t
#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

#include <opusfile.h>

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

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  struct ReadOnlyFileAdapterState;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading OPUS files using libopus</summary>
  class OpusReader {

    /// <summary>Initializes a new Opus reader on the specified file</summary>
    /// <param name="file">File the reader will access</param>
    public: OpusReader(const std::shared_ptr<const VirtualFile> &file);

    /// <summary>Frees all resources owned by the Opus reader</summary>
    public: ~OpusReader();

    /// <summary>Reads the metadata from an Opus audi ofile</summary>
    /// <param name="target">Track information container that will receive the metadata</param>
    public: void ReadMetadata(TrackInfo &target);

    /// <summary>Counts the total number of frames (= samples in each channel)</summary>
    /// <returns>The total number of frames in the audio file</returns>
    public: std::uint64_t CountTotalFrames() const;

    /// <summary>Gets the order in which interlaved samples are decoded</summary>
    /// <returns>A list of channels in the order they are interleaved</returns>
    public: std::vector<ChannelPlacement> GetChannelOrder() const;

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
    /// <param name="target">Buffer into which the samples will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    /// <remarks>
    ///   This method is invoked if a target format other than float is requested,
    ///   performing an SSE2 SIMD-enhanced conversion of the samples to the requested
    ///   target type at the decoded block level.
    /// </remarks>
    private: template<typename TSample>
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
    /// <summary>Holds the function pointers to the file I/O functions</summary>
    /// <remarks>
    ///   libopusfile takes a pointer to these, so we try to err on the side of caution
    ///   and keep the structure around for as long as the opus file is opened.
    /// </remarks>
    private: ::OpusFileCallbacks fileCallbacks;
    /// <summary>State (emulated file cursor, errors) of the virtual file adapter</summary>
    private: std::unique_ptr<ReadOnlyFileAdapterState> state;
    /// <summary>Manages the state and decoder state of the opened Opus file</summary>
    private: std::shared_ptr<::OggOpusFile> opusFile;

    /// <summary>Number of channels in the Opus file</summary>
    private: std::size_t channelCount;
    /// <summary>Index of the audio frame that will be decoded next</summary>
    private: std::uint64_t frameCursor;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  inline void OpusReader::DecodeInterleaved(TSample *target, std::size_t frameCount) {
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

  template<> void OpusReader::DecodeInterleaved<std::uint8_t>(
    std::uint8_t *target, std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<std::int16_t>(
    std::int16_t *target, std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<std::int32_t>(
    std::int32_t *target, std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<float>(
    float *target, std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeInterleaved<double>(
    double *target, std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  inline void OpusReader::DecodeSeparated(TSample *targets[], std::size_t frameCount) {
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

  template<> void OpusReader::DecodeSeparated<std::uint8_t>(
    std::uint8_t *targets[], std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<std::int16_t>(
    std::int16_t *targets[], std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<std::int32_t>(
    std::int32_t *targets[], std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<float>(
    float *targets[], std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

  template<> void OpusReader::DecodeSeparated<double>(
    double *targets[], std::size_t frameCount
  );

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H
