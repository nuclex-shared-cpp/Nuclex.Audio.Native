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

#include "./VorbisDetection.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "./VorbisVirtualFileAdapter.h"

#include <vorbis/vorbisfile.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Does absolutely nothing with a VirtualFile instance</summary>
  void doNothing(const Nuclex::Audio::Storage::VirtualFile *) {}

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  bool Detection::DoesFileExtensionSayOgg(const std::string &extension) {
    bool extensionSaysOgg;

    // Vorbis audio files generally use the file extension, .ogg,
    // as all standlone Vorbis files are wrapped in an OGG container.
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 3) { // extension with dot or long name possible
        extensionSaysOgg = (
          ((extension[0] == 'o') || (extension[0] == 'O')) &&
          ((extension[1] == 'g') || (extension[1] == 'G')) &&
          ((extension[2] == 'g') || (extension[2] == 'G'))
        );
      } else if(extensionLength == 4) { // extension with dot and long name possible
        extensionSaysOgg = (
          (extension[0] == '.') &&
          ((extension[1] == 'o') || (extension[1] == 'O')) &&
          ((extension[2] == 'g') || (extension[2] == 'G')) &&
          ((extension[3] == 'g') || (extension[3] == 'G'))
        );
      } else {
        extensionSaysOgg = false;
      }
    }

    return extensionSaysOgg;

  }

  // ------------------------------------------------------------------------------------------- //

  bool Detection::CheckIfVorbisHeaderPresent(const VirtualFile &source) {
    std::uint64_t size = source.GetSize();
    if(size < SmallestPossibleVorbisSize) {
      return false; // File is too small to be an Ogg Vorbis file
    }

    // The ::ov_test_callbacks() method needs a complete setup of I/O callbacks
    // to fetch data. I don't want to change all detection methods to shared_ptrs
    // for their virtual files, so we're just temporarily wrapping the reference
    // to the virtual file in a shared_ptr with an empty deleter.
    ::ov_callbacks fileCallbacks;
    std::shared_ptr<const VirtualFile> wrappedSource(&source, &doNothing);
    std::unique_ptr<ReadOnlyFileAdapterState> state = (
      FileAdapterFactory::CreateAdapterForReading(wrappedSource, fileCallbacks)
    );
    ::OggVorbis_File unusedFile;

    // This can return OV_FALSE (too little data), OP_EFAULT (out of memory),
    // OP_EIMPL (format uses unsupported features), OP_ENOTFORMAT (wrong file format),
    // OP_EVERSION (file format version not supported) and OP_EBADHEADER (damaged header).
    //
    // We're only interested in figuring out whether the file can be loaded:
    int result = ::ov_test_callbacks(state.get(), &unusedFile, nullptr, 0, fileCallbacks);
    if(result == 0) {
      ::ov_clear(&unusedFile);
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool Detection::CheckIfVorbisHeaderPresentLite(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleVorbisSize) {
      return false; // File is too small to be an Ogg Vorbis file
    }

    std::byte fileHeader[48];
    source.ReadAt(0, 48, fileHeader);

    // Ogg containers can contain multiple streams, potentially mixing multiple audio
    // track and even video tracks. Theoretically, the packets identifying a Vorbis
    // stream could be buried later in the file.
    //
    // We only deal with standalone Ogg Vorbis files and it seems we have a guarantee
    // that, within the Vorbis stream, the identification packet is mandatory:
    //
    // But:
    //   "A Vorbis bitstream begins with three header packets. The header packets are,
    //   in order, the identification header, the comments header, and the setup header.
    //   All are required for decode compliance. An end-of-packet condition during decoding
    //   the first or third header packet renders the stream undecodable.
    //   End-of-packet decoding the comment header is a non-fatal error condition."
    //
    // Ogg frame: https://www.xiph.org/ogg/doc/framing.html
    // Vorbis packet: https://xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-590004
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
      (fileHeader[0] == std::byte(0x4f)) &&    //  1 O | Oggs (FourCC magic header)
      (fileHeader[1] == std::byte(0x67)) &&    //  2 g |
      (fileHeader[2] == std::byte(0x67)) &&    //  3 g | All Vorbis files are OGG containers,
      (fileHeader[3] == std::byte(0x53)) &&    //  4 s | so first we should see the OGG FourCC.
      (fileHeader[4] == std::byte(0x0)) &&     //  - stream_structure version (use range?)
      (
        (fileHeader[5] == std::byte(0x2)) ||   //  - 2 = first page of logical bitstream
        (fileHeader[5] == std::byte(0x6))      //  - 6 = first and last page of logical bitstream
      ) &&
      (encodedSampleCount < 0x691200000ULL) && // total samples encoded at this point, ideally 0
      (pageSequenceNumber == 0) &&
      (
        (fileHeader[28] == std::byte(0x01)) || //  - 1 = identification header packet
        (fileHeader[28] == std::byte(0x03)) || //  - 3 = comment header packet
        (fileHeader[28] == std::byte(0x05))    //  - 5 = setup header packet
      ) &&
      (fileHeader[29] == std::byte(0x76)) &&   //  1 v | Vorbis (magic header)
      (fileHeader[30] == std::byte(0x6f)) &&   //  2 o |
      (fileHeader[31] == std::byte(0x72)) &&   //  3 r | This declares the subframe type
      (fileHeader[32] == std::byte(0x62)) &&   //  4 b | within the Ogg container
      (fileHeader[33] == std::byte(0x69)) &&   //  5 i |
      (fileHeader[34] == std::byte(0x73))      //  6 s |
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
