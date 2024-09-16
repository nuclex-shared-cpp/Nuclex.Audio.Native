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

#include "Nuclex/Audio/ContainerInfo.h" // for ContainerInfo
#include "../EndianReader.h" // for LittleEndianReader / BigEndianReader

#include <cstdint> // for std::uint8_t
#include <type_traits> // for std::is_same

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility to read the data structures found in Waveform files</summary>
  class WaveformReader {

    /// <summary>Guesses the most likely channel placement given a channel count</summary>
    /// <param name="channelCount">Number of channels in the Waveform audio file</param>
    /// <returns>The likely placement of the channels</returns>
    public: static ChannelPlacement GuessChannelPlacement(std::size_t channelCount);

    /// <summary>Checks if the FourCC of a chunk indicates the 'fmt ' chunk</summary>
    /// <param name="buffer">Buffer that will be checked for holding the 'fmt ' chunk</param>
    /// <returns>True if the buffer contained the FourCC of the 'fmt ' chunk</returns>
    public: static bool IsFormatChunk(const std::uint8_t *buffer);

    /// <summary>Checks if the FourCC of a chunk indicates the 'fact' chunk</summary>
    /// <param name="buffer">Buffer that will be checked for holding the 'fact' chunk</param>
    /// <returns>True if the buffer contained the FourCC of the 'fact' chunk</returns>
    public: static bool IsFactChunk(const std::uint8_t *buffer);

    /// <summary>Checks if the FourCC of a chunk indicates the 'data' chunk</summary>
    /// <param name="buffer">Buffer that will be checked for holding the 'data' chunk</param>
    /// <returns>True if the buffer contained the FourCC of the 'data' chunk</returns>
    public: static bool IsDataChunk(const std::uint8_t *buffer);

    /// <summary>Initializes a new Waveform audio reader</summary>
    /// <param name="target">TrackInfo structure metadata will be placed in</param>
    public: WaveformReader(Nuclex::Audio::TrackInfo &target);

    /// <summary>Checks whether the minimum required chunks have been parsed</summary>
    /// <returns>True if the minimum required chunks were present</returns>
    public: bool IsComplete() const {
      // We *should* check for factChunkParsed since it's a requirement in new waveform
      // audio files, but there's so much software that doesn't write it that we can't.
      return (
        this->formatChunkParsed &&
        (this->firstSampleOffset != std::uint64_t(-1))
      );
    }

    /// <summary>Parses the information stored in the audio format ('fmt ') chunk</summary>
    /// <typeparam name="TReader">
    ///   Reader used to read numeric values in the file as big or little endian
    /// </typeparam>
    /// <param name="buffer">
    ///   Buffer containing a format chunk, starting at its FourCC header
    /// </param>
    /// <param name="chunkLength">Length of the chunk in bytes, minus 8 bytes</param>
    public: template<typename TReader = LittleEndianReader>
    void ParseFormatChunk(const std::uint8_t *buffer, std::size_t chunkLength);

    /// <summary>Parses the information stored in the extra metadata ('fact') chunk</summary>
    /// <typeparam name="TReader">
    ///   Reader used to read numeric values in the file as big or little endian
    /// </typeparam>
    /// <param name="buffer">
    ///   Buffer containing a fact chunk, starting at its FourCC header
    /// </param>
    public: template<typename TReader = LittleEndianReader>
    void ParseFactChunk(const std::uint8_t *buffer);

    /// <summary>Records the offset of the 'data' chunk in the Waveform file</summary>
    /// <param name="startOffset">Absolute offset of the data chunk's header</param>
    /// <param name="remainingByteCount">Number of bytes that remain in the file</param>
    /// <remarks>
    ///   If this is the last chunk in the file (and <see cref="SetAfterDataChunkStart" />
    ///   is never called, then the remaining bytes in the file will be used as the number
    ///   of bytes the Waveform file's audio data is long.
    /// </remarks>
    public: void SetDataChunkStart(std::uint64_t startOffset, std::uint64_t remainingByteCount);

    /// <summary>Actual implementation of the ParseFormatChunk() method</summary>
    /// <typeparam name="TReader">
    ///   Reader used to read numeric values in the file as big or little endian
    /// </typeparam>
    /// <param name="buffer">
    ///   Buffer containing a format chunk, starting at its FourCC header
    /// </param>
    /// <param name="chunkLength">Length of the chunk in bytes, minus 8 bytes</param>
    private: template<typename TReader>
    void parseFormatChunkInternal(const std::uint8_t *buffer, std::size_t chunkLength);

    /// <summary>Actual implementation of the ParseFactChunk() method</summary>
    /// <typeparam name="TReader">
    ///   Reader used to read numeric values in the file as big or little endian
    /// </typeparam>
    /// <param name="buffer">
    ///   Buffer containing a fact chunk, starting at its FourCC header
    /// </param>
    private: template<typename TReader>
    void parseFactChunkInternal(const std::uint8_t *buffer);

    /// <summary>Calculates the playback duration of the audio data</summary>
    private: void calculateDuration();

    /// <summary>Track information container data will be stored in</summary>
    private: Nuclex::Audio::TrackInfo &target;
    /// <summary>Whether the format metadata chunk has been parsed yet</summary>
    private: bool formatChunkParsed;
    /// <summary>Whether the extra metadata chunk has been parsed yet</summary>
    private: bool factChunkParsed;

    /// <summaery>Number of bits used to store each audio sample in the file</summary>
    private: std::size_t storedBitsPerSample;
    /// <summary>Size of one frame (one sample of each channel in a row)</summary>
    private: std::size_t blockAlignment;
    /// <summary>Index of the first audio sample in the file</summary>
    private: std::uint64_t firstSampleOffset;
    /// <summary>Index one past the the last audio sample in the file</summary>
    private: std::uint64_t afterLastSampleOffset;

  };

  // ------------------------------------------------------------------------------------------- //

  inline bool WaveformReader::IsFormatChunk(const std::uint8_t *buffer) {
    return (
      (buffer[0] == 0x66) &&  //  1 f | "fmt " (audio format chunk)
      (buffer[1] == 0x6d) &&  //  2 m |
      (buffer[2] == 0x74) &&  //  3 t | This is one of the chunks that must appear in
      (buffer[3] == 0x20)     //  4   | a Waveform audio file, containing metadata.
    );
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool WaveformReader::IsFactChunk(const std::uint8_t *buffer) {
    return (
      (buffer[0] == 0x66) &&  //  1 f | "fact" (extra metadata chunk)
      (buffer[1] == 0x61) &&  //  2 a |
      (buffer[2] == 0x63) &&  //  3 c | Chunk that must appear in second generation
      (buffer[3] == 0x74)     //  4 t | Waveform audio files, storing their length.
    );
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool WaveformReader::IsDataChunk(const std::uint8_t *buffer) {
    return (
      (buffer[0] == 0x64) &&  //  1 d | "data" (audio data chunk)
      (buffer[1] == 0x61) &&  //  2 a |
      (buffer[2] == 0x74) &&  //  3 t | This chunk is mandatory for Waveform audio files
      (buffer[3] == 0x61)     //  4 a | and contains the actual audio data in one block.
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TReader>
  inline void WaveformReader::ParseFormatChunk(
    const std::uint8_t *buffer, std::size_t chunkLength
  ) {
    static_assert(
      std::is_same<TReader, LittleEndianReader>::value ||
      std::is_same<TReader, BigEndianReader>::value,
      u8"TReader must be either a LittleEndianReader or a BigEndianReader"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFormatChunk<LittleEndianReader>(
    const std::uint8_t *buffer, std::size_t chunkLength
  );

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFormatChunk<BigEndianReader>(
    const std::uint8_t *buffer, std::size_t chunkLength
  );

  // ------------------------------------------------------------------------------------------- //

  template<typename TReader>
  inline void WaveformReader::ParseFactChunk(const std::uint8_t *buffer) {
    static_assert(
      std::is_same<TReader, LittleEndianReader>::value ||
      std::is_same<TReader, BigEndianReader>::value,
      u8"TReader must be either a LittleEndianReader or a BigEndianReader"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFactChunk<LittleEndianReader>(const std::uint8_t *buffer);

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFactChunk<BigEndianReader>(const std::uint8_t *buffer);

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform

#endif // NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H
