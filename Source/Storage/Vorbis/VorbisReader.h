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

#ifndef NUCLEX_AUDIO_STORAGE_VORBIS_VORBISREADER_H
#define NUCLEX_AUDIO_STORAGE_VORBIS_VORBISREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "Nuclex/Audio/ChannelPlacement.h"

#include <string> // for std::string
#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint64_t
#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

#include <vorbis/vorbisfile.h>

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

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  struct ReadOnlyFileAdapterState;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading Ogg Vorbis files using libvorbisfile</summary>
  class VorbisReader {

    /// <summary>Initializes a new Vorbis reader on the specified file</summary>
    /// <param name="file">File the reader will access</param>
    public: VorbisReader(const std::shared_ptr<const VirtualFile> &file);

    /// <summary>Frees all resources owned by the Vorbis reader</summary>
    public: ~VorbisReader();

    /// <summary>Reads the metadata from an Vorbis audi ofile</summary>
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
    void decodeSeparatedConvertAndInterleave(TSample *target, std::size_t frameCount);

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
    void decodeSeparatedAndConvert(TSample *targets[], std::size_t frameCount);

    /// <summary>File the reader is accessing</summary>
    private: std::shared_ptr<const VirtualFile> file;
    /// <summary>Holds the function pointers to the file I/O functions</summary>
    /// <remarks>
    ///   libvorbisfile takes a pointer to these, so we try to err on the side of caution
    ///   and keep the structure around for as long as the Vorbis file is opened.
    /// </remarks>
    private: ::ov_callbacks fileCallbacks;
    /// <summary>State (emulated file cursor, errors) of the virtual file adapter</summary>
    private: std::unique_ptr<ReadOnlyFileAdapterState> state;
    /// <summary>Manages the state and decoder state of the opened Vorbis file</summary>
    private: std::shared_ptr<::OggVorbis_File> vorbisFile;

    /// <summary>Number of channels in the Vorbis file</summary>
    private: std::size_t channelCount;
    /// <summary>Index of the audio frame that will be decoded next</summary>
    private: std::uint64_t frameCursor;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)

#endif // NUCLEX_AUDIO_STORAGE_VORBIS_VORBISREADER_H
