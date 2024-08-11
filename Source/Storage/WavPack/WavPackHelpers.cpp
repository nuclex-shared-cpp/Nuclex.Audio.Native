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

#include "./WavPackHelpers.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::DoesFileExtensionSayWv(const std::string &extension) {
    bool extensionSaysWv;

    // Microsoft Waveform audio files can have the extension .wav or (rarely) .wave.
    // We'll consider both here.
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 2) { // extension without dot possible
        extensionSaysWv = (
          ((extension[0] == 'w') || (extension[0] == 'W')) &&
          ((extension[1] == 'v') || (extension[1] == 'V'))
        );
      } else if(extensionLength == 3) { // extension with dot or long name possible
        extensionSaysWv = (
          (extension[0] == '.') &&
          ((extension[1] == 'w') || (extension[1] == 'W')) &&
          ((extension[2] == 'v') || (extension[2] == 'V'))
        );
      } else {
        extensionSaysWv = false;
      }
    }

    return extensionSaysWv;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::CheckIfWavPackHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleWavPackSize) {
      return false; // File is too small to be a .wav file
    }

    std::uint8_t fileHeader[16];
    source.ReadAt(0, 16, fileHeader);

    // This checks all headers and magic numbers that are mandatory and makes sure
    // that there is at least one sub-chunk with a valid length - it will probably
    // always be the "fmt" chunk, but we don't want to be the only library that
    // has trouble with a non-standard Waveformat audio file everyone else can load.
    return (
      (fileHeader[0] == 0x77) && //  1 wvpk (file type id)
      (fileHeader[1] == 0x76) && //  2
      (fileHeader[2] == 0x70) && //  3
      (fileHeader[3] == 0x6b) && //  4
      (                          //  - uint32 with block size. *Should* be below 1 MiB.
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 4) < 0x01000000) // 16 MiB
      ) &&
      (                          //  - uint32 for version needed to decode
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8) >= 0x400) &&
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8) < 0x999)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
