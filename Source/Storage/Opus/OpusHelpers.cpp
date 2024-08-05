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

#include "./OpusHelpers.h"
#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::DoesFileExtensionSayOpus(const std::string &extension) {
    bool extensionSaysOpus;

    // FLAC audio generally only uses one file extension, .flac
    // Unless it's wrapped in an OGG container, but that's not the business of this codec
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 4) { // extension with dot or long name possible
        extensionSaysOpus = (
          ((extension[0] == 'o') || (extension[0] == 'O')) &&
          ((extension[1] == 'p') || (extension[1] == 'P')) &&
          ((extension[2] == 'u') || (extension[2] == 'U')) &&
          ((extension[3] == 's') || (extension[3] == 'S'))
        );
      } else if(extensionLength == 5) { // extension with dot and long name possible
        extensionSaysOpus = (
          (extension[0] == '.') &&
          ((extension[1] == 'o') || (extension[1] == 'O')) &&
          ((extension[2] == 'p') || (extension[2] == 'P')) &&
          ((extension[3] == 'u') || (extension[3] == 'U')) &&
          ((extension[4] == 's') || (extension[4] == 'S'))
        );
      } else {
        extensionSaysOpus = false;
      }
    }

    return extensionSaysOpus;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Helpers::CheckIfOpusHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleOpusSize) {
      return false; // File is too small to be a .opus file
    }

    std::uint8_t fileHeader[48];
    source.ReadAt(0, 48, fileHeader);

    // OPUS files, even those produced by the standalone opusenc executable,
    // are .ogg files with an OPUS stream inside.
    //
    // Now this library has two paths, either via libopus and its built-in .ogg streaming
    // facilities or via the (as of yet unwritten) container support that will provide
    // the OPUS stream directly out of the .mp3, .mka or .ogg container.
    //
    // But:
    //   "The first packet in the logical Ogg stream MUST contain the identification header,
    //   which uniquely identifies a stream as Opus audio. It MUST begin with
    //   the 8 bytes "OpusHead". It MUST be placed alone in the first page of the logical
    //   Ogg stream. This page MUST have the ’beginning of stream’ flag set.""
    //
    // Ogg fame: https://www.xiph.org/ogg/doc/framing.html
    // OPUS packet: https://wiki.xiph.org/OggOpus#Packet_Organization
    //
    return (
      (fileHeader[0] == 0x4f) && //  1 Oggs (magic header)
      (fileHeader[1] == 0x67) && //  2
      (fileHeader[2] == 0x67) && //  3
      (fileHeader[3] == 0x53) && //  4
      (fileHeader[4] == 0x0) &&  //  - stream_structure version (currently 0 - use range?)
      (fileHeader[5] == 0x2) &&  //  - 2 = first page of logical bitstream (= untruncated)
      (                          //  - uint64, total samples encoded at this point, should be 0
        (*reinterpret_cast<const std::uint64_t *>(fileHeader + 6) < 0x100000000ULL)
      ) &&
      (                          //  - uint32, page sequence number (0 if untruncated file)
        (*reinterpret_cast<const std::uint32_t *>(fileHeader + 18) == 0)
      ) &&
      (fileHeader[28] == 0x4f) && //  1 OpusHead (magic header)
      (fileHeader[29] == 0x70) && //  2
      (fileHeader[30] == 0x75) && //  3
      (fileHeader[31] == 0x73) && //  4
      (fileHeader[32] == 0x48) && //  5
      (fileHeader[33] == 0x65) && //  6
      (fileHeader[34] == 0x61) && //  7
      (fileHeader[35] == 0x64) && //  8
      (fileHeader[36] == 0x01)    //  - version
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus
