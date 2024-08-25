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

#include "./WaveformReader.h"

#include "Nuclex/Audio/Errors/CorruptedFileError.h"
#include "Nuclex/Audio/Errors/UnsupportedFormatError.h"

#include <array> // for std::array, used a 'Guid'
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A UUID is just a very long ID value with 128 bits / or 16 bytes</summary>
  typedef std::array<std::uint8_t, 16> Guid;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Audio format that indicates uncompressed PCM audio</summary>
  constexpr std::uint16_t WaveFormatPcm = 1;
  /// <summary>Audio format that indicates uncompressed floating point PCM audio</summary>
  constexpr std::uint16_t WaveFormatFloatPcm = 3;
  /// <summary>Audio format tag that indicates a WAVEFORMATEXTENSIBLe header</summary>
  constexpr std::uint16_t WaveFormatExtensible = 65534;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>GUID for the integer PCM audio subformat in WAVEFORMATEXTENSIBLE</summary>
  const Guid WaveFormatSubTypePcm = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
  };

  /// <summary>GUID for the float PCM audio subformat in WAVEFORMATEXTENSIBLE</summary>
  const Guid WaveFormatSubTypeIeeeFloat = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement WaveformReader::GuessChannelPlacement(std::size_t channelCount) {
    if(channelCount == 8) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight |
        ChannelPlacement::FrontCenter | ChannelPlacement::LowFrequencyEffects |
        ChannelPlacement::SideLeft | ChannelPlacement::SideRight |
        ChannelPlacement::BackLeft | ChannelPlacement::BackRight
      );
    } else if(channelCount == 6) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight |
        ChannelPlacement::FrontCenter | ChannelPlacement::LowFrequencyEffects |
        ChannelPlacement::BackLeft | ChannelPlacement::BackRight
      );
    } else if(channelCount == 5) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight |
        ChannelPlacement::BackLeft | ChannelPlacement::BackRight |
        ChannelPlacement::LowFrequencyEffects
      );
    } else if(channelCount == 4) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight |
        ChannelPlacement::BackLeft | ChannelPlacement::BackRight
      );
    } else if(channelCount == 3) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight |
        ChannelPlacement::LowFrequencyEffects
      );
    } else if(channelCount == 2) {
      return (
        ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight
      );
    } else if(channelCount == 1) {
      return ChannelPlacement::FrontCenter;
    } else {
      return ChannelPlacement::Unknown;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  WaveformReader::WaveformReader(Nuclex::Audio::TrackInfo &target) :
    target(target),
    formatChunkParsed(false),
    factChunkParsed(false),
    afterDataChunkParsed(false),
    blockAlignment(0),
    firstSampleOffset(std::uint64_t(-1)),
    afterLastSampleOffset(std::uint64_t(-1)) {}

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFormatChunk<LittleEndianReader>(
    const std::uint8_t *buffer, std::size_t chunkLength
  ) {
    parseFormatChunkInternal<LittleEndianReader>(buffer, chunkLength);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFormatChunk<BigEndianReader>(
    const std::uint8_t *buffer, std::size_t chunkLength
  ) {
    parseFormatChunkInternal<BigEndianReader>(buffer, chunkLength);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFactChunk<LittleEndianReader>(const std::uint8_t *buffer) {
    parseFactChunkInternal<LittleEndianReader>(buffer);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformReader::ParseFactChunk<BigEndianReader>(const std::uint8_t *buffer) {
    parseFactChunkInternal<BigEndianReader>(buffer);
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformReader::SetDataChunkStart(
    std::uint64_t startOffset, std::uint64_t remainingByteCount
  ) {
    if(this->firstSampleOffset != std::uint64_t(-1)) {
      throw Errors::CorruptedFileError(
        u8"Waveform audio file contains more than one 'data' (audio data) chunk"
      );
    }

    this->firstSampleOffset = startOffset + 8;
    this->afterLastSampleOffset = startOffset + remainingByteCount;

    if(this->formatChunkParsed) {
      calculateDuration();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformReader::SetPostDataChunkStart(std::uint64_t startOffset) {
    assert(
      (this->firstSampleOffset != std::uint64_t(-1)) &&
      u8"Post-'data' chunk offset is set after encountering the 'data' chunk first"
    );
    this->afterLastSampleOffset = startOffset;

    if(this->formatChunkParsed) {
      calculateDuration();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TReader>
  void WaveformReader::parseFormatChunkInternal(
    const std::uint8_t *chunk, std::size_t chunkLength
  ) {
    if(this->formatChunkParsed) {
      throw Errors::CorruptedFileError(
        u8"Waveform audio file contains more than one 'fmt ' (metadata) chunk"
      );
    }

    // These fields are safe to read without checking the length since this method is only
    // ever invoked if there are at least enough bytes for one full WAVEFORMAT header.
    std::uint16_t formatTag = TReader::ReadUInt16(chunk + 8);
    this->target.ChannelCount = static_cast<std::size_t>(
      TReader::ReadUInt16(chunk + 10)
    );
    this->target.SampleRate = static_cast<std::size_t>(
      TReader::ReadUInt32(chunk + 12)
    );
    //std::uint32_t bytesPerSecond = TReader::ReadUInt32(chunk + 16);
    this->blockAlignment = static_cast<std::size_t>(
      TReader::ReadUInt16(chunk + 20)
    );

    // Any further data in the 'fmt ' chunk depends on the format tag. In the earliest
    // revisions, 'WAVEFORMAT' only had the 'wBitsPerSample' field if the 'wFormatTag'
    // was set to 'WAVE_FORMAT_PCM', but this was later retconned to be 'WAVEFORMATEX'
    // and made mandatory for all present and future format tags.
    if((formatTag == WaveFormatPcm) || (formatTag == WaveFormatFloatPcm)) {

      if(chunkLength < 22 - 8) {
        throw Nuclex::Audio::Errors::CorruptedFileError(
          u8"Waveform audio file claims PCMWAVEFORMAT or WAVEFORMATEX header, "
          u8"but 'fmt ' (metadata) chunk size is too small"
        );
      }

      // We've got a PCMWAVEFORMAT or WAVEFORMATEX header, so bits per sample must be
      // a multiple of 8. We'll still parse it expecting nonconformant values.
      this->target.BitsPerSample = static_cast<std::size_t>(
        TReader::ReadUInt16(chunk + 22)
      );
      if(this->target.BitsPerSample >= 33) {
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_64;
      } else if(formatTag == WaveFormatFloatPcm) { // Float and not 64 bits? Must be 32.
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_32;
      } else if(this->target.BitsPerSample >= 25) {
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
      } else if(this->target.BitsPerSample >= 17) {
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
      } else if(this->target.BitsPerSample >= 9) {
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
      } else {
        this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
      }

      // In the original Waveform audio format specification, only 1 or 2 channels
      // were mentioned (without explicitly stating that more would be in violation).
      // Either way, nobody cares, our only problem with more channels is that their
      // placements are unknown. We do some guesswork here.
      this->target.ChannelPlacements = WaveformReader::GuessChannelPlacement(this->target.ChannelCount);

    } else if(formatTag == WaveFormatExtensible) {

      if(chunkLength != 40) {
        throw Nuclex::Audio::Errors::CorruptedFileError(
          u8"Waveform audio file claims WAVEFORMATEXTENSIBLE header, "
          u8"but 'fmt ' (metadata) chunk size doesn't match"
        );
      }

      // This would hold the stored number of bits per sample. It should be redundant,
      // unless some 
      //std::uint16_t bitsPerSample = TReader::ReadUInt16(chunk + 22);

      // According to Microsoft:
      //
      //   "For the WAVEFORMATEXTENSIBLE structure, the Format.cbSize field must be set
      //   to 22 and the SubFormat field must be set to KSDATAFORMAT_SUBTYPE_PCM."
      //
      std::uint16_t extraParameterLength = TReader::ReadUInt16(chunk + 24);
      if(extraParameterLength != 22) {
        throw Nuclex::Audio::Errors::CorruptedFileError(
          u8"Waveform audio file claims WAVEFORMATEXTENSIBLE header, "
          u8"but extra parameter size violates file format specification"
        );
      }

      // This field holds the 'valid' bits per sample (can be any number, the remaining
      // bits up to the next byte are zero-padded in each sample).
      this->target.BitsPerSample = static_cast<std::size_t>(
        TReader::ReadUInt16(chunk + 26)
      );
      this->target.ChannelPlacements = static_cast<Nuclex::Audio::ChannelPlacement>(
        TReader::ReadUInt32(chunk + 28)
      );

      Guid audioFormatSubType;
      std::copy_n(chunk + 32, 16, audioFormatSubType.data());

      if(audioFormatSubType == WaveFormatSubTypePcm) {
        if(this->target.BitsPerSample >= 25) {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
        } else if(this->target.BitsPerSample >= 17) {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
        } else if(this->target.BitsPerSample >= 9) {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
        } else {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
        }
      } else if(audioFormatSubType == WaveFormatSubTypeIeeeFloat) {
        if(this->target.BitsPerSample >= 33) {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_64;
        } else {
          this->target.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_32;
        }
      } else {
        throw Nuclex::Audio::Errors::UnsupportedFormatError(
          u8"Waveform audio file uses WAVEFORMATEXTENSIBLE with a format "
          u8"sub-type that isn't supported (only PCM and float are supported)."
        );
      }

    } else {
      throw Nuclex::Audio::Errors::UnsupportedFormatError(
        u8"Waveform audio file contains data in an unsupported format. "
        u8"Only PCM and floating point PCM data formats are supported."
      );
    }

    this->formatChunkParsed = true;

    if(this->firstSampleOffset != std::uint64_t(-1)) {
      calculateDuration();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TReader>
  void WaveformReader::parseFactChunkInternal(const std::uint8_t *chunk) {
    if(this->factChunkParsed) {
      throw Errors::CorruptedFileError(
        u8"Waveform audio file contains more than one 'fact' (extra metadata) chunk"
      );
    }

    // Our dilemma:
    //
    // This chunk became mandatory for the "new wave format," aka the format everyone is
    // one since 1994. Except that almost no application respects this.
    //
    // It's also only useful for validation, but to know the actual (playable) length of
    // the Waveform audio file, we have to look at the data chunk and the number of bytes
    // that come after it.
    std::uint32_t sampleCount = TReader::ReadUInt32(chunk + 8);
    (void)sampleCount;

    this->factChunkParsed = true;
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformReader::calculateDuration() {
    assert(
      this->formatChunkParsed &&
      u8"calculateDuration() is called with the format chunk already parsed"
    );
    assert(
      (this->firstSampleOffset != std::uint64_t(-1)) &&
      u8"calculateDuration() is called with the data chunk extents known"
    );

    std::uint64_t totalSampleCount = (
      (this->afterLastSampleOffset - this->firstSampleOffset) / this->blockAlignment
    );
    this->target.Duration = std::chrono::microseconds(
      totalSampleCount * 1'000'000 / this->target.SampleRate
    );
  }


  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
