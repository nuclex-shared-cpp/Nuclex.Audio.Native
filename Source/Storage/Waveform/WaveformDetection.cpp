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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "./WaveformDetection.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  bool Detection::DoesFileExtensionSayWav(const std::string &extension) {
    bool extensionSaysWav;

    // Microsoft Waveform audio files can have the extension .wav or (rarely) .wave.
    // We'll consider both here.
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 3) { // extension without dot possible
        extensionSaysWav = (
          ((extension[0] == 'w') || (extension[0] == 'W')) &&
          ((extension[1] == 'a') || (extension[1] == 'A')) &&
          ((extension[2] == 'v') || (extension[2] == 'V'))
        );
      } else if(extensionLength == 4) { // extension with dot or long name possible
        extensionSaysWav = (
          (extension[0] == '.') &&
          ((extension[1] == 'w') || (extension[1] == 'W')) &&
          ((extension[2] == 'a') || (extension[2] == 'A')) &&
          ((extension[3] == 'v') || (extension[3] == 'V'))
        );
        extensionSaysWav |= (
          ((extension[0] == 'w') || (extension[0] == 'W')) &&
          ((extension[1] == 'a') || (extension[1] == 'A')) &&
          ((extension[2] == 'v') || (extension[2] == 'V')) &&
          ((extension[3] == 'e') || (extension[3] == 'E'))
        );
      } else if(extensionLength == 5) { // extension with dot and long name possible
        extensionSaysWav = (
          (extension[0] == '.') &&
          ((extension[1] == 'w') || (extension[1] == 'W')) &&
          ((extension[2] == 'a') || (extension[2] == 'A')) &&
          ((extension[3] == 'v') || (extension[3] == 'V')) &&
          ((extension[4] == 'e') || (extension[4] == 'E'))
        );
      } else {
        extensionSaysWav = false;
      }
    }

    return extensionSaysWav;
  }

  // ------------------------------------------------------------------------------------------- //

  // This specification is so unclear.
  //
  // The 1994 IBM + Microsoft document on RIFF with the Waveformat Audio Format says
  // in its text only that the "fmt" chunk *must* come before the "data" chunk.
  //
  // But the file structure specification seems to go further and indicate that
  // the "fmt" chunk must be the *first* chunk overall:
  //
  //   <WAVE-form> 🡒
  //     RIFF( 'WAVE'
  //       <fmt-ck> // Format
  //       [<fact-ck>] // Fact chunk
  //       [<cue-ck>] // Cue points
  //       [<playlist-ck>] // Playlist
  //       [<assoc-data-list>] // Associated data list
  //       <wave-data> ) // Wave data
  //
  // Behavior of 5 randomly sampled libraries that load WAVE files on GitHub:
  //
  //   * mhroth/tinywav -> skips chunks until "fmt" chunk is found
  //   * adamstark/AudioFile -> loads entire file, skips chunks until "fmt" chunk found
  //   * mackron/dr_libs -> anticipates a "ds64" chunk before the "fmt" chunk for RF64,
  //                        then skips over chunks until the "fmt" chunk is found
  //   * mackron/miniaudio -> skips chunks until "fmt" chunk is found
  //   * wcaleniekubaa/wave -> only handles RIFF(?), fmt, data, skips until "fmt" chunk found
  //
  // We don't want to scan through the whole file to detect its type,
  // so instead we check for everything else (RIFF file, WAVE data and that *some* chunk,
  // not necessarily the "fmt" chunk, begins after the header).
  // 

  bool Detection::CheckIfWaveformHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleWaveformSize) {
      return false; // File is too small to be a .wav file
    }

    std::uint8_t fileHeader[24];
    source.ReadAt(0, 24, fileHeader);

    // Officially, there's only RIFF (little-endian) and RIFX (big-endian) with identical
    // structure except for the endianness of any integers / floats found in the file.
    // Some libraries I sampled also handle "FFIR" (RIFF backwards) files as big-endian,
    // and rarely, even anticipate "XFIR" files. Whether these exist in the wild, it's
    // a tiny step to support all possible variants, so we do.
    bool isLittleEndian = (
      (fileHeader[0] == 0x52) &&   //  1 R | RIFF (fourcc; chunk descriptor)
      (fileHeader[1] == 0x49) &&   //  2 I |
      (fileHeader[2] == 0x46) &&   //  3 F | This is the standard fourcc for little-endian
      (fileHeader[2] == 0x46)      //  4 F | Waveform audio files.
    ) || (
      (fileHeader[0] == 0x58) &&   //  1 X | XFIR (fourcc; chunk descriptor)
      (fileHeader[1] == 0x46) &&   //  2 F |
      (fileHeader[2] == 0x49) &&   //  3 I | This would be an opposite-endian RIFX file
      (fileHeader[2] == 0x52)      //  4 R | written by a confused audio library :)
    );

    bool isBigEndian = (
      (fileHeader[0] == 0x52) &&   //  1 R | RIFX (fourcc; chunk descriptor)
      (fileHeader[1] == 0x49) &&   //  2 I |
      (fileHeader[2] == 0x46) &&   //  3 F | The official fourcc for big-endian
      (fileHeader[2] == 0x58)      //  4 X | Waveform audio files
    ) || (
      (fileHeader[0] == 0x46) &&   //  1 F | FFIR (fourcc; chunk descriptor)
      (fileHeader[1] == 0x46) &&   //  2 F |
      (fileHeader[2] == 0x49) &&   //  3 I | This would be produced by a endian-unaware
      (fileHeader[2] == 0x52)      //  4 R | library saving a file on a big-endian system.
    );

    std::uint32_t blockSize, firstChunkSize;
    if(isLittleEndian) {
      blockSize = (
        (static_cast<std::uint32_t>(fileHeader[4])) |
        (static_cast<std::uint32_t>(fileHeader[5]) << 8) |
        (static_cast<std::uint32_t>(fileHeader[6]) << 16) |
        (static_cast<std::uint32_t>(fileHeader[7]) << 24)
      );
      firstChunkSize = (
        (static_cast<std::uint32_t>(fileHeader[16])) |
        (static_cast<std::uint32_t>(fileHeader[17]) << 8) |
        (static_cast<std::uint32_t>(fileHeader[18]) << 16) |
        (static_cast<std::uint32_t>(fileHeader[19]) << 24)
      );
    } else if(isBigEndian) {
      blockSize = (
        (static_cast<std::uint32_t>(fileHeader[4]) << 24) |
        (static_cast<std::uint32_t>(fileHeader[5]) << 16) |
        (static_cast<std::uint32_t>(fileHeader[6]) << 8) |
        (static_cast<std::uint32_t>(fileHeader[7]))
      );
      firstChunkSize = (
        (static_cast<std::uint32_t>(fileHeader[16]) << 24) |
        (static_cast<std::uint32_t>(fileHeader[17]) << 16) |
        (static_cast<std::uint32_t>(fileHeader[18]) << 8) |
        (static_cast<std::uint32_t>(fileHeader[19]))
      );
    } else {
      return false;
    }

    return (
      (blockSize >= 36) &&
      (blockSize < 0x80000000) &&
      (fileHeader[8] == 0x57) &&  //  1 W | WAVE (format id)
      (fileHeader[9] == 0x41) &&  //  2 A |
      (fileHeader[10] == 0x56) && //  3 V | RIFF is a chunked format for many purposes,
      (fileHeader[11] == 0x45) && //  4 E | we're looking for a RIFF file with audio data.
      (firstChunkSize < 0x80000000) // Size of unknown first chunk
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
