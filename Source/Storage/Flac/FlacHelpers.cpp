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
    bool extensionSaysFlac;

    // FLAC audio generally only uses one file extension, .flac
    // Unless it's wrapped in an OGG container, but that's not the business of this codec
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 4) { // extension with dot or long name possible
        extensionSaysFlac = (
          ((extension[0] == 'f') || (extension[0] == 'F')) &&
          ((extension[1] == 'l') || (extension[1] == 'L')) &&
          ((extension[2] == 'a') || (extension[2] == 'A')) &&
          ((extension[3] == 'c') || (extension[3] == 'C'))
        );
      } else if(extensionLength == 5) { // extension with dot and long name possible
        extensionSaysFlac = (
          (extension[0] == '.') &&
          ((extension[1] == 'f') || (extension[1] == 'F')) &&
          ((extension[2] == 'l') || (extension[2] == 'L')) &&
          ((extension[3] == 'a') || (extension[3] == 'A')) &&
          ((extension[4] == 'c') || (extension[4] == 'C'))
        );
      } else {
        extensionSaysFlac = false;
      }
    }

    return extensionSaysFlac;
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
    // https://xiph.org/flac/format.html#def_STREAMINFO
    //

    // These values are big endian, so we reconstitute them from their 3 bytes manually.
    // While the following should be correct on both little and big endian, I do
    // not have access to a big endian system for testing (time for a RISC-V VM?)
    std::uint32_t metaDataBlockLength = (
      (static_cast<std::uint32_t>(fileHeader[5]) << 16) |
      (static_cast<std::uint32_t>(fileHeader[6]) << 8) |
      (static_cast<std::uint32_t>(fileHeader[7]))
    );
    std::uint16_t minimumBlockSize = (
      (static_cast<std::uint16_t>(fileHeader[8]) << 8) |
      (static_cast<std::uint16_t>(fileHeader[9]))
    );
    std::uint16_t maximumBlockSize = (
      (static_cast<std::uint16_t>(fileHeader[10]) << 8) |
      (static_cast<std::uint16_t>(fileHeader[11]))
    );

    return (
      (fileHeader[0] == 0x66) && //  1 fLaC (magic header)
      (fileHeader[1] == 0x4c) && //  2
      (fileHeader[2] == 0x61) && //  3
      (fileHeader[3] == 0x43) && //  4
      (fileHeader[4] == 0x0) &&  //  1 metadata header (0 = streaminfo block + more follow)
      (                          //  - uint32, contains 24-bit metadata block length
        (metaDataBlockLength >= 34) // metadata block is at least 34 bytes long
      ) &&
      (                          //  - uint16, minimum block size in streaminfo block
        (minimumBlockSize >= 16)
      ) &&
      (                          //  - uint16, maximum block size in streaminfo block
        (maximumBlockSize >= 16)
      ) &&
      (                          //  - maximum block size must at least be minimum bloc ksize
        (maximumBlockSize >= minimumBlockSize)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac
