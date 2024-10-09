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

    /// <summary>Sets the reader up to decode audio samples</summary>
    /// <remarks>
    ///   By default, the WaveformReader only initializes its internal data structures to
    ///   be able to read metadata. Before calling the Read...() methods, this method
    ///   needs to be called once. It will complete the set up and enable the WaveformReader
    ///   to read and convert audio samples.
    /// </remarks>
    public: void PrepareForReading();

    /// <summary>Reads samples from the audio file in interleaved format</summary>
    /// <typeparam name="TSample">Type of samples that will be decoded</typeparam>
    /// <param name="target">Buffer into which the samples will be written</param>
    /// <param name="startFrame">Frame from which on to read samples</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    public: template<typename TSample>
    void ReadInterleaved(TSample *target, std::uint64_t startFrame, std::size_t frameCount);

    /// <summary>Reads samples from the audio file in interleaved format</summary>
    /// <typeparam name="TSample">Type of samples that will be decoded</typeparam>
    /// <param name="targets">Buffers into which the channels will be written</param>
    /// <param name="startFrame">Frame from which on to read samples</param>
    /// <param name="frameCount">Number of frame that should be decoded</param>
    public: template<typename TSample>
    void ReadSeparated(TSample *targets[], std::uint64_t startFrame, std::size_t frameCount);

    /// <summary>Reads samples from the audio file and converts them</summary>
    /// <typeparam name="TSample">Type of samples to convert to</typeparam>
    /// <typeparam name="BitsPerSampleOver16">
    ///   Whether the *stored* bits per sample is above 16 (whether the output
    ///   type has more than 16 bits is trivial to determine via sizeof())
    /// </typeparam>
    /// <typeparam name="WidenFactor">
    ///   How many times the *stored* bits have to be repeated to fill the output.
    ///   -2: Data is double, factor does not apply
    ///   -1: Data is float, factor does not apply
    ///    0: Data needs to be truncated
    ///   +1: Exact match, only copy
    ///   +2: Bit pattern needs to be repeated once
    ///   +3: Bit pattern needs to be tripled
    /// </typeparam>
    /// <param name="target">Buffer into which the samples will be written</param>
    /// <param name="startFrame">Frame from which on to read samples</param>
    /// <param name="frameCount">Number of frames that should be read</param>
    private: template<
      typename TSample, // output type
      bool BitsPerSampleOver16 = false, // for decoded bits per sample, not TSample
      int WidenFactor = 0 // set negative if decoded samples are float/double!
    >
    void readInterleavedAndConvert(
      TSample *target, std::uint64_t startFrame, std::size_t frameCount
    );

    /// <summary>Reads samples from the audio file and converts them</summary>
    /// <typeparam name="TSample">Type of samples to convert to</typeparam>
    /// <typeparam name="BitsPerSampleOver16">
    ///   Whether the *stored* bits per sample is above 16 (whether the output
    ///   type has more than 16 bits is trivial to determine via sizeof())
    /// </typeparam>
    /// <typeparam name="WidenFactor">
    ///   How many times the *stored* bits have to be repeated to fill the output.
    ///   -2: Data is double, factor does not apply
    ///   -1: Data is float, factor does not apply
    ///    0: Data needs to be truncated
    ///   +1: Exact match, only copy
    ///   +2: Bit pattern needs to be repeated once
    ///   +3: Bit pattern needs to be tripled
    /// </typeparam>
    /// <param name="targets">Buffers into which the channels will be written</param>
    /// <param name="startFrame">Frame from which on to read samples</param>
    /// <param name="frameCount">Number of frames that should be read</param>
    private: template<
      typename TSample, // output type
      bool BitsPerSampleOver16 = false, // for decoded bits per sample, not TSample
      int WidenFactor = 0 // set negative if decoded samples are float/double!
    >
    void readInterleavedConvertAndSeparate(
      TSample *targets[], std::uint64_t startFrame, std::size_t frameCount
    );

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

    /// <summary>Signature for the interleaved decode method</summary>
    private: template<typename TSample>
    using ReadInterleavedFunction = void (WaveformReader::*)(
      TSample *, std::uint64_t, std::size_t
    );

    /// <summary>Signature for the channel-separated decode method</summary>
    private: template<typename TSample>
    using ReadSeparatedFunction = void (WaveformReader::*)(
      TSample *[], std::uint64_t, std::size_t
    );

    /// <summary>Reads the opened file's data to interleaved 8-bit integers</summary>
    private: ReadInterleavedFunction<std::uint8_t> readInterleavedUint8;
    /// <summary>Reads the opened file's data to interleaved 16-bit integers</summary>
    private: ReadInterleavedFunction<std::int16_t> readInterleavedInt16;
    /// <summary>Reads the opened file's data to interleaved 32-bit integers</summary>
    private: ReadInterleavedFunction<std::int32_t> readInterleavedInt32;
    /// <summary>Reads the opened file's data to interleaved floats</summary>
    private: ReadInterleavedFunction<float> readInterleavedFloat;
    /// <summary>Reads the opened file's data to interleaved doubles</summary>
    private: ReadInterleavedFunction<double> readInterleavedDouble;
    /// <summary>Reads the opened file's data to separate 8-bit channels</summary>
    private: ReadSeparatedFunction<std::uint8_t> readSeparatedUint8;
    /// <summary>Reads the opened file's data to separate 16-bit channels</summary>
    private: ReadSeparatedFunction<std::int16_t> readSeparatedInt16;
    /// <summary>Reads the opened file's data to separate 32-bit channels</summary>
    private: ReadSeparatedFunction<std::int32_t> readSeparatedInt32;
    /// <summary>Reads the opened file's data to separate float channels</summary>
    private: ReadSeparatedFunction<float> readSeparatedFloat;
    /// <summary>Reads the opened file's data to separate double channels</summary>
    private: ReadSeparatedFunction<double> readSeparatedDouble;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform

#endif // NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H
