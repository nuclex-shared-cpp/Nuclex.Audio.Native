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
  // Behavior of 5 random libraries that load WAVE files on GitHub:
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

    // This checks all headers and magic numbers that are mandatory and makes sure
    // that there is at least one sub-chunk with a valid length - it will probably
    // always be the "fmt" chunk, but we don't want to be the only library that
    // has trouble with a non-standard Waveformat audio file everyone else can load.
    return (
      (fileHeader[0] == 0x52) && //  1 RIFF (file type id)
      (fileHeader[1] == 0x49) && //  2
      (fileHeader[2] == 0x46) && //  3
      (fileHeader[3] == 0x46) && //  4
      (                          //  - uint32 for total size minus 8 bytes
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 4) >= 36) &&
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 4) < 0x80000000)
      ) &&
      (fileHeader[8] == 0x57) &&  // 1 WAVE (format id)
      (fileHeader[9] == 0x41) &&  // 2
      (fileHeader[10] == 0x56) && // 3
      (fileHeader[11] == 0x45) && // 4
      (                          //  - uint32 with size of unknown first chunk
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 16) < 0x80000000)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
