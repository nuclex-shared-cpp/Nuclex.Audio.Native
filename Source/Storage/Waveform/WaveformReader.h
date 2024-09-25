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

#ifndef NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H
#define NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/TrackInfo.h"

#include "../EndianReader.h" // for LittleEndianReader / BigEndianReader

#include <optional> // for std::optional
#include <memory> // for std::shared_ptr
#include <vector> // for std::vector

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  struct ContainerInfo;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility to read the data structures found in Waveform files</summary>
  class WaveformReader {

    /// <summary>Reads only the metadata from a suspected Waveform file</summary>
    /// <param name="file">File from which the metadata will be read if possible</param>
    /// <returns>The metadata if the virtual file was indeed a Waveform file</returns>
    /// <remarks>
    ///   This method is different from creating a WaveformReader and using its
    ///   <see cref="ReadMetadata" /> method in that it will return an empty result
    ///   in case the file does not look like a Waveform file. If, on the other hand,
    ///   this methid is fairly sure to be dealing with a Waveform file but it has
    ///   structural errors, an exception will be thrown still.
    /// </remarks>
    public: static std::optional<ContainerInfo> TryReadMetadata(
      const std::shared_ptr<const VirtualFile> &file
    );

    /// <summary>Initializes a new Opus reader on the specified file</summary>
    /// <param name="source">File the reader will access</param>
    public: WaveformReader(const std::shared_ptr<const VirtualFile> &source);

    /// <summary>Frees all resources owned by the Opus reader</summary>
    public: ~WaveformReader();

    /// <summary>Reads the metadata from an Opus audi ofile</summary>
    /// <param name="target">Track information container that will receive the metadata</param>
    public: void ReadMetadata(TrackInfo &target);

    /// <summary>Counts the total number of frames (= samples in each channel)</summary>
    /// <returns>The total number of frames in the audio file</returns>
    public: std::uint64_t CountTotalFrames() const;

    /// <summary>Gets the order in which interlaved samples are decoded</summary>
    /// <returns>A list of channels in the order they are interleaved</returns>
    public: std::vector<ChannelPlacement> GetChannelOrder() const;

    //public: void ReadFrames()

    /// <summary>File the reader is accessing</summary>
    private: std::shared_ptr<const VirtualFile> file;
    /// <summary>Whether the file's data is in little endian format</summary>
    private: bool isLittleEndian;
    /// <summary>Collected metadata on the opened audio file</summary>
    private: TrackInfo trackInfo;
    /// <summary>Offset of the first audio sample in the file</summary>
    private: std::uint64_t firstSampleOffset;
    /// <summary>Total number of audio samples in the file</summary>
    private: std::uint64_t totalFrameCount;
    /// <summary>Number of bytes consumed per audio frame</summary>
    private: std::size_t bytesPerFrame;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform

#endif // NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H
