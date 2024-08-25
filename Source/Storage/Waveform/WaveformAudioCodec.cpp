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
#include "./WaveformReader.h"

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "Nuclex/Audio/Errors/CorruptedFileError.h"
#include "Nuclex/Audio/Errors/UnsupportedFormatError.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the read buffer when loading audio data from a Waveform file</summary>
  constexpr std::size_t ReadBufferSize = 16384;

  /// <summary>Bytes to read initially to get the 'fmt ' chunk in the best case</summary>
  /// <remarks>
  ///   The RIFF (WAVE) format is chunked. According to the specification, the 'fmt ' chunk
  ///   should be the first chunk (with room for interpretation), so we'll try to read enough
  ///   data to grab the whole 'fmt ' chunk, if it is indeed at the promised location.
  /// </remarks>
  constexpr std::size_t OptimistcInitialByteCount = 60;

  /// <summary>Length of the (ancient) legacy WAVEFORMAT chunk</summary>
  constexpr std::size_t WaveFormatChunkLengthWithHeader = 22;

  /// <summary>Length of the WAVEFORMATEXTENSIBLE chunk</summary>
  /// <remarks>
  ///   Unless the audio file uses WAVEFORMATEX with its variable-length field (in which case
  ///   we're not interested in the extra data since we can't interpret it), this is
  ///   the maximum size of the 'fmt ' chunk. Note that WAVEFORMATEXTENSIBLE does away with
  ///   the variable-length field again by stating that it has to have a length of exactly
  ///   22 bytes.
  /// </remarks>
  constexpr std::size_t WaveFormatExtensibleChunkLengthWithHeader = 48;

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

  /// <summary>Throws an exception if a chunk in the Waveform audio file is too short</summary>
  /// <param name="recordedChunkLengthWithHeader">Chunk length the file has recorded</param>
  /// <param name="minimumChunkLengthWithHeader">Smallest valid chunk length</param>
  /// <param name="readByteCount">Chunk length we were actually able to read</param>
  /// <param name="chunkName">Name of the chunk, used in the potential exception message</param>
  void requireChunkLength(
    std::size_t recordedChunkLengthWithHeader,
    std::size_t minimumChunkLengthWithHeader,
    std::size_t readByteCount,
    const std::string_view &chunkName
  ) {
    if(recordedChunkLengthWithHeader < minimumChunkLengthWithHeader) {
      std::string message(u8"Waveform audio file contains too short ", 39);
      message.append(chunkName);
      //throw Nuclex::Audio::Errors::CorruptedFileError(message);
    }
    if(readByteCount < recordedChunkLengthWithHeader) {
      std::string message(u8"Waveform audio file truncated, ", 31);
      message.append(chunkName);
      message.append(u8" is truncated", 13);
      //throw Nuclex::Audio::Errors::CorruptedFileError(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads chunks from the file until locating the 'fmt ' chunk</summary>
  /// <typeparam name="TReader">Endian-specific data Reader matching the file</typeparam>
  /// <param name="source">File from which the data will be read</param>
  /// <param name="fileSize">Size of the input file in bytes</param>
  /// <param name="buffer">Buffer large enough to hold the format chunk</param>
  /// <param name="readByteCount">Number of bytes in the buffer and buffer size</param>
  /// <returns>
  ///   Information about the audio data in the file or nothing if the file is not
  ///   a Waveform audio file
  /// </returns>
  template<typename TReader>
  std::optional<Nuclex::Audio::ContainerInfo> scanChunksAndFillContainerInfo(
    const std::shared_ptr<const Nuclex::Audio::Storage::VirtualFile> &source,
    std::uint64_t fileSize,
    std::uint8_t *buffer /*[OptimistcInitialByteCount]*/,
    std::size_t readByteCount
  ) {
    using Nuclex::Audio::Storage::Waveform::WaveformReader;
    assert(
      (readByteCount >= (WaveFormatChunkLengthWithHeader + 12)) &&
      u8"Call to scanChunksAndFillContainerInfo() starts with enough data for one chunk header"
    );

    // The RIFF format states the size of the whole file (minus the 8 bytes from the four-cc
    // and the length field itself). It's probably not a good idea to require it to match
    // the precise, actual file size because some tool might have appended tagging information
    // or the file could be truncated, so we just use it to ignore any data trailing the file.
    {
      std::uint32_t expectedFileSize = TReader::ReadUInt32(buffer + 4);
      //if(expectedFileSize >= 0x8000000) { // Waveform files are limited to 2 GiB. Or not?
      //  return std::optional<Nuclex::Audio::ContainerInfo>();
      //}
      if(static_cast<std::uint64_t>(expectedFileSize) + 8 < fileSize) {
        fileSize = static_cast<std::uint64_t>(expectedFileSize) + 8;
      }
    }

    // Next is the format id. RIFF containers are also used for video data (.avi) and some
    // other things, so we need to make sure ours says 'WAVE' for Waveform audio.
    {
      bool formatIsWaveform = (
        (buffer[8] == 0x57) &&  //  1 W | WAVE (format id)
        (buffer[9] == 0x41) &&  //  2 A |
        (buffer[10] == 0x56) && //  3 V | RIFF is a chunked format for many purposes,
        (buffer[11] == 0x45)    //  4 E | we're looking for a RIFF file with audio data.
      );
      if(!formatIsWaveform) {
        return std::optional<Nuclex::Audio::ContainerInfo>();
      }
    }

    // Advance to the first chunk start. On the downside, we lose 12 bytes from the buffer,
    // on the upside, we don't need them (since the buffer is guaranteed to still cover
    // one WAVEFORMATEXTENSIBLE) and it saves us from using a secondary buffer pointer.
    std::uint64_t readOffset = 12;
    {
      buffer += readOffset;
      readByteCount -= readOffset;
    }

    // Scan for the 'fmt ' (format; metadata) chunk, 'fact' and 'data' chunk to determine
    // channel count, sample rate, data format and (from the latter two) duration.
    Nuclex::Audio::ContainerInfo containerInfo;
    containerInfo.DefaultTrackIndex = 0;
    WaveformReader reader(containerInfo.Tracks.emplace_back());
    for(;;) {
      std::uint32_t chunkLength = TReader::ReadUInt32(buffer + 4);
      std::size_t chunkLengthWithHeader = chunkLength + 8;

      // For the 'fmt ' chunk, we require the entire chunk to be present, up to the length
      // of its WaveFormatExtensible variant (which is the maximum length we'll ever read)
      if(WaveformReader::IsFormatChunk(buffer)) {
        if(WaveFormatExtensibleChunkLengthWithHeader < chunkLengthWithHeader) {
          chunkLengthWithHeader = WaveFormatExtensibleChunkLengthWithHeader;
        }
        requireChunkLength(
          chunkLengthWithHeader,
          WaveFormatChunkLengthWithHeader,
          readByteCount,
          std::string_view(u8"'fmt ' (metadata) chunk", 23)
        );
        reader.ParseFormatChunk<TReader>(buffer, chunkLength);
      } else if(WaveformReader::IsFactChunk(buffer)) {
        requireChunkLength(
          chunkLengthWithHeader, 8 + 4, readByteCount,
          std::string_view(u8"'fact' (extra meatdata) chunk", 29)
        );
        reader.ParseFactChunk<TReader>(buffer, chunkLength);
      } else if(WaveformReader::IsDataChunk(buffer)) {
        reader.ParseDataChunk<TReader>(buffer, chunkLength);
      }

      // Skip to the next chunk. Chunks are 16-bit aligned, but this alignment is not recorded
      // in the chunk length field inside the chunk itself. Thus, if the chunk length is
      // an odd number of bytes, it will be padded with a zero byte, requiring adjustment.
      readOffset += chunkLengthWithHeader + (chunkLength & 1);
      if(fileSize < readOffset + WaveFormatChunkLengthWithHeader) {
        break; // File would end before a (complete) 'fmt ' chunk can arrive
      }

      // Figure out how many bytes to read next. We'll try to read enough bytes to
      // grab a potential WAVEFORMATEXTENSIBLE chunk in one go, but will be happy
      // with fewer bytes if the file ends earlier.
      readByteCount = WaveFormatExtensibleChunkLengthWithHeader;
      if(fileSize < readOffset + readByteCount) {
        readByteCount = fileSize - readOffset;
      }
      source->ReadAt(readOffset, readByteCount, buffer);
    }

    // If all required chunks have been found, we have the needed data and
    // can provide to the caller. Otherwise, at least one of the needed chunks
    // was missing and the file is considered broken. We only decide this here
    // so that our behavior regarding duplicate chunks is consistent and not
    // dependent on their appearance in a certain order.
    if(reader.IsComplete()) {
      return containerInfo;
    } else {
      throw Nuclex::Audio::Errors::CorruptedFileError(
        u8"Waveform audio file was missing one ore more mandatory information chunks"
      );
    }
  }

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
    (void)extensionHint;

    // First, check the size of the file. We need this to avoid going outside the file
    // bounds during our scan and if the file is too small, we can exit early.
    std::uint64_t fileSize = source->GetSize();
    if(fileSize < SmallestPossibleWaveformSize) {
      return std::optional<ContainerInfo>();
    }

    // Calculate the number of bytes for the initial read in which we will check the FourCC
    // and RIFF / Waveform header before going on the hunt for sub-chunks.
    std::size_t readByteCount;
    if(fileSize < OptimistcInitialByteCount) {
      readByteCount = static_cast<std::size_t>(fileSize);
    } else {
      readByteCount = OptimistcInitialByteCount;
    }

    std::vector<std::uint8_t> buffer(readByteCount);
    source->ReadAt(0, readByteCount, buffer.data());

    // Figure out what kind of file we're dealing with
    FourCC fourCC = checkFourCC(buffer.data());
    if((fourCC == FourCC::Riff) || (fourCC == FourCC::Xfir)) {
      return scanChunksAndFillContainerInfo<LittleEndianReader>(
        source, fileSize, buffer.data(), readByteCount
      );
    } else if((fourCC == FourCC::Rifx) || (fourCC == FourCC::Ffir)) {
      return scanChunksAndFillContainerInfo<BigEndianReader>(
        source, fileSize, buffer.data(), readByteCount
      );
    } else {
      return std::optional<ContainerInfo>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
