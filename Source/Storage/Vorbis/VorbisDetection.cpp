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

// This header is in /usr/include/opus/opusfile.h on my Linux system,
// But directly under libopusfile/include/ in the libopus sources,
// which we use, so this #include statement may differ from other code examples.
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
      return false; // File is too small to be an .opus file
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
      return false; // File is too small to be a .opus file
    }

    std::uint8_t fileHeader[48];
    source.ReadAt(0, 48, fileHeader);

    // VORBIS files, even those produced by the standalone opusenc executable,
    // are .ogg files with an VORBIS stream inside.
    //
    // Now this library has two paths, either via libopus and its built-in .ogg streaming
    // facilities or via the (as of yet unwritten) container support that will provide
    // the VORBIS stream directly out of the .mpa, .mka or .ogg container.
    //
    // But:
    //   "The first packet in the logical Ogg stream MUST contain the identification header,
    //   which uniquely identifies a stream as Vorbis audio. It MUST begin with
    //   the 8 bytes "VorbisHead". It MUST be placed alone in the first page of the logical
    //   Ogg stream. This page MUST have the ’beginning of stream’ flag set.""
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
      (fileHeader[0] == 0x4f) &&  //  1 O | Oggs (FourCC magic header)
      (fileHeader[1] == 0x67) &&  //  2 g |
      (fileHeader[2] == 0x67) &&  //  3 g | All Vorbis audio files are OGG containers,
      (fileHeader[3] == 0x53) &&  //  4 s | so the first thing we should see is an OGG FourCC
      (fileHeader[4] == 0x0) &&   //  - stream_structure version (currently 0 - use range?)
      (
        (fileHeader[5] == 0x2) || //  - 2 = first page of logical bitstream (= file start intact)
        (fileHeader[5] == 0x6)    //  - 6 = first and also last page of logical bitstream
      ) &&
      (encodedSampleCount < 0x691200000ULL) && // total samples encoded at this point, ideally 0
      (pageSequenceNumber == 0) &&
      (
        (fileHeader[28] == 1) ||  //  - 1 = identification header packet
        (fileHeader[28] == 3) ||  //  - 3 = comment header packet
        (fileHeader[28] == 5)     //  - 5 = setup header packet
      ) &&
      (fileHeader[29] == 0x76) && //  1 v | Vorbis (magic header)
      (fileHeader[30] == 0x6f) && //  2 o |
      (fileHeader[31] == 0x72) && //  3 r |
      (fileHeader[32] == 0x62) && //  4 b |
      (fileHeader[33] == 0x69) && //  5 i |
      (fileHeader[34] == 0x73)    //  6 s |
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
