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

#include <string> // for std::string
#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint64_t
#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

#include <opusfile.h>

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  class TrackInfo;

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

    /// <summary>Decodes sample from the audio file in interleaved format</summary>
    /// <param name="buffer">Buffer into which the samples will be written</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    public: void DecodeInterleaved(float *buffer, std::size_t frameCount);

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

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSREADER_H
