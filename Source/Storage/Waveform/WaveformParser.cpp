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

#include "./WaveformParser.h"

#include "Nuclex/Audio/Errors/CorruptedFileError.h"
#include "Nuclex/Audio/Errors/UnsupportedFormatError.h"

#include <array> // for std::array, used a 'Guid'
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A UUID is just a very long ID value with 128 bits / or 16 bytes</summary>
  typedef std::array<std::byte, 16> Guid;

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
    std::byte(0x01), std::byte(0x00), std::byte(0x00), std::byte(0x00),
    std::byte(0x00), std::byte(0x00), std::byte(0x10), std::byte(0x00),
    std::byte(0x80), std::byte(0x00), std::byte(0x00), std::byte(0xaa),
    std::byte(0x00), std::byte(0x38), std::byte(0x9b), std::byte(0x71)
  };

  /// <summary>GUID for the float PCM audio subformat in WAVEFORMATEXTENSIBLE</summary>
  const Guid WaveFormatSubTypeIeeeFloat = {
    std::byte(0x03), std::byte(0x00), std::byte(0x00), std::byte(0x00),
    std::byte(0x00), std::byte(0x00), std::byte(0x10), std::byte(0x00),
    std::byte(0x80), std::byte(0x00), std::byte(0x00), std::byte(0xaa),
    std::byte(0x00), std::byte(0x38), std::byte(0x9b), std::byte(0x71)
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement WaveformParser::GuessChannelPlacement(std::size_t channelCount) {
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

  WaveformParser::WaveformParser(Nuclex::Audio::TrackInfo &target) :
    target(target),
    formatChunkParsed(false),
    factChunkParsed(false),
    storedBitsPerSample(0),
    blockAlignment(0),
    firstSampleOffset(std::uint64_t(-1)),
    afterLastSampleOffset(std::uint64_t(-1)) {}

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformParser::ParseFormatChunk<LittleEndianReader>(
    const std::byte *buffer, std::size_t chunkLength
  ) {
    parseFormatChunkInternal<LittleEndianReader>(buffer, chunkLength);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformParser::ParseFormatChunk<BigEndianReader>(
    const std::byte *buffer, std::size_t chunkLength
  ) {
    parseFormatChunkInternal<BigEndianReader>(buffer, chunkLength);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformParser::ParseFactChunk<LittleEndianReader>(const std::byte *buffer) {
    parseFactChunkInternal<LittleEndianReader>(buffer);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  void WaveformParser::ParseFactChunk<BigEndianReader>(const std::byte *buffer) {
    parseFactChunkInternal<BigEndianReader>(buffer);
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformParser::SetDataChunkStart(
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

  std::size_t WaveformParser::CountBytesPerFrame() {
    assert(
      this->formatChunkParsed &&
      u8"CountBytesPerFrame() is called with the format chunk already parsed"
    );

    // We have 'storedBitsPerSample' which already specifies the number of bytes one frame
    // should use, but the Waveform audio file format provides a 'blockAlignment' field.
    // The specification is rather muddy about this:
    //
    //   "Playback software needs to process a multiple of wBlockAlign bytes of data at
    //   a time, so the value of wBlockAlign can be used for buffer alignment"
    //
    // That leaves room for 'wBlockAlign' to be one frame, two frames, the whole file even.
    // The "ADPCM" format, for example, mandates bigger, power-of-two block alignments
    // (but it has an actual concept of blocks), other formats use values of '0' or '1' to
    // indicate that data is not aligned to blocks.
    //
    // Regarding plain PCM or FLOAT audio files, here is what 5 randomly sampled libraries do:
    //
    //   mhroth/tinywav:  .BlockAlign = numChannels * tw->sampFmt;
    //   adamstark/AudioFile:  numBytesPerBlock = getNumChannels() * (bitDepth / 8);
    //   evpobr/libsndwave:  ..._writef (psf, "22", BHW2 (psf->bytewidth * psf->sf.channels)
    //   audionamix/wave:  header.fmt.byte_per_block = bytes_per_sample * channel_number;
    //   Signalsmith-Audio/audio-wav-example:  write16(file, channels*bytesPerSample);
    //
    // So the assumption that 'wBlockAlign' is (bytesPerSample x channelCount) is relatively
    // safe. We'll still be a little bit defensive and use a fallback for invalid values:
    //
    std::size_t bytesPerFrame = (
      (this->storedBitsPerSample + 7) / 8 * this->target.ChannelCount
    );
    if(this->blockAlignment >= bytesPerFrame) {
      return this->blockAlignment;
    } else {
      return bytesPerFrame;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WaveformParser::GetAudioDataOffset() {
    assert(
      (this->firstSampleOffset != std::uint64_t(-1)) &&
      u8"GetAudioDataOffset() is called with the data chunk extents known"
    );

    return this->firstSampleOffset;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WaveformParser::CountFrames() {
    assert(
      this->formatChunkParsed &&
      u8"CountFrames() is called with the format chunk already parsed"
    );
    assert(
      (this->firstSampleOffset != std::uint64_t(-1)) &&
      u8"CountFrames() is called with the data chunk extents known"
    );

    std::size_t bytesPerFrame = CountBytesPerFrame();
    return (this->afterLastSampleOffset - this->firstSampleOffset) / bytesPerFrame;
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TReader>
  void WaveformParser::parseFormatChunkInternal(
    const std::byte *chunk, std::size_t chunkLength
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
      // a multiple of 8. To be safe, we'll still parse it expecting nonconformant values.
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
      // Either way, in the wild west of existing Waveform implementations, nobody cares,
      // our only problem with more channels is we have to guess their placements.
      this->target.ChannelPlacements = (
        GuessChannelPlacement(this->target.ChannelCount)
      );

    } else if(formatTag == WaveFormatExtensible) {

      if(chunkLength != 40) {
        throw Nuclex::Audio::Errors::CorruptedFileError(
          u8"Waveform audio file claims WAVEFORMATEXTENSIBLE header, "
          u8"but 'fmt ' (metadata) chunk size doesn't match"
        );
      }

      // This offset holds the number of *stored* bits per sample (whereas the next
      // bits per sample value, below, is the number of *used* bits per sample).
      this->storedBitsPerSample = static_cast<std::size_t>(
        TReader::ReadUInt16(chunk + 22)
      );

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

      // This field holds the *used* bits per sample (which can be any number,
      // the remaining bits up to the next byte are zero-padded in each sample).
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
  void WaveformParser::parseFactChunkInternal(const std::byte *chunk) {
    if(this->factChunkParsed) {
      throw Errors::CorruptedFileError(
        u8"Waveform audio file contains more than one 'fact' (extra metadata) chunk"
      );
    }

    // Our dilemma:
    //
    // This chunk became mandatory for the "new wave format," aka the format everyone is
    // using since 1994. Except that almost no application actually respects this.
    //
    // It's also only useful for validation, but to know the actual (playable) length of
    // the Waveform audio file, we have to look at the data chunk and the number of bytes
    // that come after it.
    //
    std::uint32_t sampleCount = TReader::ReadUInt32(chunk + 8);
    (void)sampleCount;

    this->factChunkParsed = true;
  }

  // ------------------------------------------------------------------------------------------- //

  void WaveformParser::calculateDuration() {
    assert(
      this->formatChunkParsed &&
      u8"calculateDuration() is called with the format chunk already parsed"
    );

    this->target.Duration = std::chrono::microseconds(
      CountFrames() * 1'000'000 / this->target.SampleRate
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
