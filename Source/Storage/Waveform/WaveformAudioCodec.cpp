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

#include "./WaveformAudioCodec.h"
#include "./WaveformDetection.h"

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>FourCCs used in Waveform files</summary>
  enum class FourCC {

    /// <summary>File started with other characters, not a Waveform file</summary>
    Other = 0,
    /// <summary>RIFF header, original little endian Waveform format</summary>
    Riff = 1,
    /// <summary>RIFX header, big endian Waveform file with identical structure</summary>
    Rifx = 2,
    /// <summary>FFIR header, big endian Waveform file with identical structure</summary>
    Ffir = 3,
    /// <summary>XFIR header, little endian Waveform file with identical structure</summary>
    Xfir = 4

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks the FourCC on a file for the codes relevant to Waveform files</summary>
  /// <param name="fileHeader">Array containing the bytes in the file header</param>
  /// <returns>The FourCC found in the file header</returns>
  FourCC checkFourCC(const std::uint8_t *fileHeader/*[4]*/) {
    if(
      (fileHeader[0] == 0x52) &&
      (fileHeader[1] == 0x49) &&
      (fileHeader[2] == 0x46)
    ) {
      if(fileHeader[3] == 0x46) {
        return FourCC::Riff;
      } else if(fileHeader[3] == 0x58) {
        return FourCC::Rifx;
      }
    } else if(
      (fileHeader[0] == 0x46) &&
      (fileHeader[1] == 0x46) &&
      (fileHeader[2] == 0x49) &&
      (fileHeader[3] == 0x52)
    ) {
      return FourCC::Ffir;
    } else if(
      (fileHeader[0] == 0x58) &&
      (fileHeader[1] == 0x46) &&
      (fileHeader[2] == 0x49) &&
      (fileHeader[3] == 0x52)
    ) {
      return FourCC::Xfir;
    }

    return FourCC::Other;
  }

  // ------------------------------------------------------------------------------------------- //

  //template<typename T

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  const std::string &WaveformAudioCodec::GetName() const {
    const static std::string codecName(u8"Microsoft Waveform", 18);
    return codecName;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::string> &WaveformAudioCodec::GetFileExtensions() const  {
    const static std::vector<std::string> extensions {
      std::string(u8"wav", 3),
      std::string(u8"wave", 4)
    };

    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> WaveformAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    constexpr std::size_t ReadBufferSize = 16384;

    // First, check the size of the file. We need this to avoid going outside the file
    // bounds during our scan and if the file is too small, we can exit early.
    std::uint64_t fileSize = source->GetSize();
    if(fileSize < SmallestPossibleWaveformSize) {
      return std::optional<ContainerInfo>();
    }

    // Calculate the number of bytes for the initial read in which we will check the FourCC
    // and RIFF / Waveform header before going on the hunt for sub-chunks.
    std::size_t readByteCount;
    if(fileSize < ReadBufferSize) {
      readByteCount = static_cast<std::size_t>(fileSize);
    } else {
      readByteCount = ReadBufferSize;
    }

    std::vector<std::uint8_t> buffer(ReadBufferSize);
    source->ReadAt(0, readByteCount, buffer.data());

    // Figure out what kind of file we're dealing with
    FourCC fourCC = checkFourCC(buffer.data());
    if(fourCC == FourCC::Other) {
      return std::optional<ContainerInfo>();
    }

    //std::uint32_t riffFileSize = 

    return std::optional<ContainerInfo>();
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
