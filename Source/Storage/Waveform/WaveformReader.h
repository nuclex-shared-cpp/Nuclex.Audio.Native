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

#include "Nuclex/Audio/TrackInfo.h" // for TrackInfo

#include <cstdint>

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
      (buffer[3] == 0x74)     //  4 t | Waveform audio files, storing their length..
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

}}}} // namespace Nuclex::Audio::Storage::Waveform

#endif // NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMREADER_H
