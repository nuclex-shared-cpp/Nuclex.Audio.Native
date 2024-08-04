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

#include "./FlacHelpers.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::DoesFileExtensionSayFlac(const std::string &extension) {
    bool extensionSaysWav;

    // Microsoft Waveform audio files can have the extension .wav or (rarely) .wave.
    // We'll consider both here.
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 4) { // extension with dot or long name possible
        extensionSaysWav = (
          ((extension[0] == 'f') || (extension[0] == 'F')) &&
          ((extension[1] == 'l') || (extension[1] == 'L')) &&
          ((extension[2] == 'a') || (extension[2] == 'A')) &&
          ((extension[3] == 'c') || (extension[3] == 'C'))
        );
      } else if(extensionLength == 5) { // extension with dot and long name possible
        extensionSaysWav = (
          (extension[0] == '.') &&
          ((extension[1] == 'f') || (extension[1] == 'F')) &&
          ((extension[2] == 'l') || (extension[2] == 'L')) &&
          ((extension[3] == 'a') || (extension[3] == 'A')) &&
          ((extension[4] == 'c') || (extension[4] == 'C'))
        );
      } else {
        extensionSaysWav = false;
      }
    }

    return extensionSaysWav;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::CheckIfFlacHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleFlacSize) {
      return false; // File is too small to be a .flac file
    }

    std::uint8_t fileHeader[16];
    source.ReadAt(0, 16, fileHeader);

    // FLAC specification:
    //   "At the start of a FLAC file or stream, following the fLaC ASCII file signature,
    //   one or more metadata blocks MUST be present before any audio frames appear.
    //   The first metadata block MUST be a streaminfo block."
    //
    // Metadata header (uint32):
    //   Bit 0    - is this the last metadata block yes/no
    //   Bit 1-7  - type of the metadata block
    //   Bit 8-31 - 24-bit length of metadata block in bytes
    //
    // Also:
    //   "The minimum block size and the maximum block size MUST be in the 16-65535 range.
    //   The minimum block size MUST be equal to or less than the maximum block size"
    //
    return (
      (fileHeader[0] == 0x66) && //  1 fLaC (magic header)
      (fileHeader[1] == 0x4c) && //  2
      (fileHeader[2] == 0x61) && //  3
      (fileHeader[3] == 0x43) && //  4
      (fileHeader[4] == 0x0) &&  //  1 metadata header (0 = streaminfo block + more follow)
      (                          //  - uint32, contains 24-bit metadata block length
        ((*reinterpret_cast<const std::uint32_t *>(fileHeader + 4) & 0x00FFFFFF) >= 34)
      ) &&
      (                          //  - uint16, minimum block size in streaminfo block
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8) >= 16)
      ) &&
      (                          //  - uint16, maximum block size in streaminfo block
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 10) >= 16)
      ) &&
      (                          //  - minimum block size must be smaller than maximum
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 8))
        <
        (*reinterpret_cast<const std::uint16_t *>(fileHeader + 10))
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac
