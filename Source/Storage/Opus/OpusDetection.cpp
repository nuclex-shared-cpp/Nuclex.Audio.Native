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

#include "./OpusDetection.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/Storage/VirtualFile.h"

// This header is in /usr/include/opus/opusfile.h on my Linux system,
// But directly under libopusfile/include/ in the libopus sources,
// which we use, so this #include statement may differ from other code examples.
#include <opusfile.h>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  bool Detection::DoesFileExtensionSayOpus(const std::string &extension) {
    bool extensionSaysOpus;

    // OPUS audio generally only uses one file extension, .opus.
    // All standlone OPUS files are wrapped in an OGG container, but the file extension
    // nevertheless should be .opus for single opus streams.
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

  bool Detection::CheckIfOpusHeaderPresent(const VirtualFile &source) {
    std::uint64_t size = source.GetSize();
    if(size < SmallestPossibleOpusSize) {
      return false; // File is too small to be an .opus file
    }

    // The docs state a pure Opus stream (standlone .opus file) canb detected with 57 bytes.
    // Standlone OPUS files are always wrapped in OGG, which allows other emebedded streams.
    //
    // According for the docs:
    //
    //   "Something like 512 bytes will give more reliable results for multiplexed streams."
    //
    std::byte fileHeader[512];
    std::size_t checkLength = static_cast<std::size_t>(std::min<std::uint64_t>(size, 512));

    source.ReadAt(0, checkLength, fileHeader);

    // This can return OP_FALSE (too little data), OP_EFAULT (out of memory),
    // OP_EIMPL (format uses unsupported features), OP_ENOTFORMAT (wrong file format),
    // OP_EVERSION (file format version not supported) and OP_EBADHEADER (damaged header).
    //
    // We're only interested in figuring out whether the file can be loaded:
    int result = ::op_test(
      nullptr, reinterpret_cast<unsigned char *>(fileHeader), checkLength
    );
    return (result == 0);
  }

  // ------------------------------------------------------------------------------------------- //

  bool Detection::CheckIfOpusHeaderPresentLite(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleOpusSize) {
      return false; // File is too small to be a .opus file
    }

    std::byte fileHeader[48];
    source.ReadAt(0, 48, fileHeader);

    // OPUS files, even those produced by the standalone opusenc executable,
    // are .ogg files with an OPUS stream inside.
    //
    // Now this library has two paths, either via libopus and its built-in .ogg streaming
    // facilities or via the (as of yet unwritten) container support that will provide
    // the OPUS stream directly out of the .mpa, .mka or .ogg container.
    //
    // But:
    //   "The first packet in the logical Ogg stream MUST contain the identification header,
    //   which uniquely identifies a stream as Opus audio. It MUST begin with
    //   the 8 bytes "OpusHead". It MUST be placed alone in the first page of the logical
    //   Ogg stream. This page MUST have the ’beginning of stream’ flag set.""
    //
    // Ogg frame: https://www.xiph.org/ogg/doc/framing.html
    // Opus packet: https://wiki.xiph.org/OggOpus#Packet_Organization
    //
    std::uint64_t encodedSampleCount = (
      (static_cast<std::uint64_t>(fileHeader[6])) |
      (static_cast<std::uint64_t>(fileHeader[7]) << 8) |
      (static_cast<std::uint64_t>(fileHeader[8]) << 16) |
      (static_cast<std::uint64_t>(fileHeader[9]) << 24) |
      (static_cast<std::uint64_t>(fileHeader[10]) << 32) |
      (static_cast<std::uint64_t>(fileHeader[11]) << 40) |
      (static_cast<std::uint64_t>(fileHeader[12]) << 48) |
      (static_cast<std::uint64_t>(fileHeader[13]) << 56)
    );
    std::uint32_t pageSequenceNumber = (
      (static_cast<std::uint32_t>(fileHeader[18])) |
      (static_cast<std::uint32_t>(fileHeader[19]) << 8) |
      (static_cast<std::uint32_t>(fileHeader[20]) << 16) |
      (static_cast<std::uint32_t>(fileHeader[21]) << 24)
    );

    return (
      (fileHeader[0] == std::byte(0x4f)) &&  //  1 O | Oggs (FourCC magic header)
      (fileHeader[1] == std::byte(0x67)) &&  //  2 g |
      (fileHeader[2] == std::byte(0x67)) &&  //  3 g | All Opus audio files are OGG containers,
      (fileHeader[3] == std::byte(0x53)) &&  //  4 s | so the first thing we should see is an OGG FourCC
      (fileHeader[4] == std::byte(0x0)) &&   //  - stream_structure version (currently 0 - use range?)
      (fileHeader[5] == std::byte(0x2)) &&   //  - 2 = first page of logical bitstream (= file start intact)
      (encodedSampleCount < 0x691200000ULL) && // total samples encoded at this point, ideally 0
      (pageSequenceNumber == 0) &&
      (fileHeader[28] == std::byte(0x4f)) && //  1 O | OpusHead (magic header)
      (fileHeader[29] == std::byte(0x70)) && //  2 p |
      (fileHeader[30] == std::byte(0x75)) && //  3 u |
      (fileHeader[31] == std::byte(0x73)) && //  4 s |
      (fileHeader[32] == std::byte(0x48)) && //  5 H |
      (fileHeader[33] == std::byte(0x65)) && //  6 e |
      (fileHeader[34] == std::byte(0x61)) && //  7 a |
      (fileHeader[35] == std::byte(0x64)) && //  8 d |
      (fileHeader[36] == std::byte(0x01))    //  - version
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
