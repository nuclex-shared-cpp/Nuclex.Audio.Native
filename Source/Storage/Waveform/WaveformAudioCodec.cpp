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
  constexpr std::size_t WaveFormatChunkLength = 22; // 8 bytes header + 14 bytes data

  /// <summary>Length of the WAVEFORMATEXTENSIBLE chunk</summary>
  /// <remarks>
  ///   Unless the audio file uses WAVEFORMATEX with its variable-length field (in which case
  ///   we're not interested in the extra data since we can't interpret it), this is
  ///   the maximum size of the 'fmt ' chunk. Note that WAVEFORMATEXTENSIBLE does away with
  ///   the variable-length field again by stating that it has to have a length of exactly
  ///   22 bytes.
  /// </remarks>
  constexpr std::size_t WaveFormatExtensibleChunkLength = 48; // 8 bytes header + 40 bytes data

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
  std::optional<Nuclex::Audio::ContainerInfo> findAndParseFormatChunk(
    const std::shared_ptr<const Nuclex::Audio::Storage::VirtualFile> &source,
    std::uint64_t fileSize,
    std::uint8_t *buffer /*[OptimistcInitialByteCount]*/,
    std::size_t readByteCount
  ) {
    using Nuclex::Audio::Storage::Waveform::WaveformReader;

    // The RIFF format states the size of the whole file (minus the 8 bytes from the four-cc
    // and the length field itself). It's probably not a good idea to check it against 
    // the precise, actual file size because some tool might have appended tagging information
    // or the file could be truncated, so we just check it for plausibility and avoid reading
    // any data that trails behind the file.
    {
      std::uint32_t expectedFileSize = TReader::ReadUInt32(buffer + 4);
      if(expectedFileSize >= 0x8000000) {
        return std::optional<Nuclex::Audio::ContainerInfo>();
      }
      if(expectedFileSize > fileSize + 8) {
        fileSize = expectedFileSize + 8;
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

    // Advance to the chunk start offset. On the downside, we lose 12 bytes from the buffer,
    // on the upside, we don't need them (since the buffer is guaranteed to still cover
    // one WAVEFORMATEXTENSIBLE) and it saves us from using a secondary buffer pointer.
    std::uint64_t readOffset = 12;
    buffer += readOffset;
    readByteCount -= readOffset;

    Nuclex::Audio::ContainerInfo containerInfo;
    containerInfo.DefaultTrackIndex = 0;
    WaveformReader reader(containerInfo.Tracks.emplace_back());

    // We can just assume that the initial read covers the first sub-chunk since
    // one of the checks done by the caller is to verify that this file has at least
    // the size that the smallest possible Waveform audio file can take.
    for(;;) {

      // Check the supposed length of this chunk. Anticipate that the file may have been
      // truncated, so if the chunk with its stated length overruns the file size,
      // we'll just bail out and complain that the file is corrupted.
      std::uint32_t chunkLength = TReader::ReadUInt32(buffer + 4);
      if(readByteCount < chunkLength) {
        break; // file was truncated and is considered corrupt
      }

      if(WaveformReader::IsFormatChunk(buffer)) {
        //trackInfo.CodecName = "Waveform";
        reader.ParseFormatChunk<TReader>(buffer, chunkLength);
        return containerInfo;
      } else if(WaveformReader::IsFactChunk(buffer)) {
        // 
      }

      // Chunks are 16-bit aligned, but this alignment does not influence the chunk length
      // field inside the chunk itself. Thus, if the chunk length is an even number of bytes,
      // it will be padded with a zero byte. We need to account for this here.
      if((chunkLength & 1) != 0) {
        ++chunkLength;
      }

      // Skip to the next chunk. If this would put us beyond the end of the file, or so close
      // to it that no (complete) 'fmt ' chunk can follow, we can only bail out.
      readOffset += chunkLength + 8;
      if(fileSize < readOffset + WaveFormatChunkLength) {
        break; // File would end before a (complete) 'fmt ' chunk can arrive
      }

      // Figure out how many bytes to read next. We'll try to read enough bytes to
      // grab a potential WAVEFORMATEXTENSIBLE chunk in one go, but will be happy
      // with fewer bytes if the file ends earlier.
      readByteCount = WaveFormatExtensibleChunkLength;
      if(fileSize < readOffset + readByteCount) {
        readByteCount = fileSize - readOffset;
      }
      source->ReadAt(readOffset, readByteCount, buffer);
    }

    // At this point, we clearly identified the file as a Waveform audio file, but it
    // was lacking the 'fmt ' chunk, which is mandatory. Thus, instead of returning
    // an empty result (which would state that it's another file type), we now throw
    // an error to indicate that this is in fact a corrupted audio file.
    throw Nuclex::Audio::Errors::CorruptedFileError(
      u8"No valid 'fmt ' (format metadata) chunk present in Waveform audio file"
    );
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
      return findAndParseFormatChunk<LittleEndianReader>(source, fileSize, buffer.data(), readByteCount);
    } else if((fourCC == FourCC::Rifx) || (fourCC == FourCC::Ffir)) {
      return findAndParseFormatChunk<BigEndianReader>(source, fileSize, buffer.data(), readByteCount);
    } else {
      return std::optional<ContainerInfo>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
