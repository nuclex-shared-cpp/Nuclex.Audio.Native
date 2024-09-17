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

#include "./WavPackDetection.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  bool Detection::DoesFileExtensionSayWv(const std::string &extension) {

    // According to its documentation, WavPack only has one appropriate extension, .wv
    {
      std::size_t extensionLength = extension.length();
      if(extensionLength == 2) { // extension without dot possible
        return (
          ((extension[0] == 'w') || (extension[0] == 'W')) &&
          ((extension[1] == 'v') || (extension[1] == 'V'))
        );
      } else if(extensionLength == 3) { // extension with dot or long name possible
        return (
          (extension[0] == '.') &&
          ((extension[1] == 'w') || (extension[1] == 'W')) &&
          ((extension[2] == 'v') || (extension[2] == 'V'))
        );
      } else {
        return false;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  bool Detection::CheckIfWavPackHeaderPresent(const VirtualFile &source) {
    if(source.GetSize() < SmallestPossibleWavPackSize) {
      return false; // File is too small to be a .wav file
    }

    std::uint8_t fileHeader[16];
    source.ReadAt(0, 16, fileHeader);

    // The WavPack block headers are entirely in little endian (see WavPack 4 and 5
    // file format specification, chapter "2.0 Block Header"), so put these integers
    // together by hand to aovid any endian mix-ups

    // Size of the entire block minus 8 bytes (the block's fourcc + this size field)
    std::uint32_t blockSize = (
      (static_cast<std::uint32_t>(fileHeader[4])) |
      (static_cast<std::uint32_t>(fileHeader[5]) << 8) |
      (static_cast<std::uint32_t>(fileHeader[6]) << 16) |
      (static_cast<std::uint32_t>(fileHeader[7]) << 24)
    );

    // Version needed to decode the file
    std::uint32_t version = (
      (static_cast<std::uint16_t>(fileHeader[8])) |
      (static_cast<std::uint16_t>(fileHeader[9]) << 8)
    );

    // This checks all headers and magic numbers that are mandatory and makes sure
    // that there is at least one sub-chunk with a valid length - it will probably
    // always be the "fmt" chunk, but we don't want to be the only library that
    // has trouble with a non-standard Waveformat audio file everyone else can load.
    return (
      (fileHeader[0] == 0x77) &&  //  1 w | ckID (fourcc; file type id)
      (fileHeader[1] == 0x76) &&  //  2 v |
      (fileHeader[2] == 0x70) &&  //  3 p |
      (fileHeader[3] == 0x6b) &&  //  4 k |
      (blockSize < 0x01000000) && // Block size: *should* be below 1 MiB, we check for 16 MiB
      (version >= 0x400) &&       // Version: expect at least version 4 for this format
      (version < 0x999)           // Version: allow up to 9.99 before assuming bad file
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
