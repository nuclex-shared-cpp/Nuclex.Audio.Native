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

#include "./FlacReader.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/TrackInfo.h"
#include "Nuclex/Audio/Errors/CorruptedFileError.h"

#include "../../Platform/FlacApi.h" // for the wrapped FLAC API functions

#include <Nuclex/Support/Text/StringHelper.h> // for StringHelper::GetTrimmed()
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat FlacReader::SampleFormatFromBitsPerSample(int bitsPerSample) {

    // I'm not entirely sure if FLAC files even can have bits per sample that are
    // not a multiple of 8, but in the sense of defensive programming, we do range checks:
    if(bitsPerSample >= 25) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
    } else if(bitsPerSample >= 17) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
    } else if(bitsPerSample >= 9) {
      return Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
    } else {
      return Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement FlacReader::ChannelPlacementFromChannelCountAndAssignment(
    std::size_t channelCount, ::FLAC__ChannelAssignment channelAssignments
  ) {
    switch(channelAssignments) {
      case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT: {
        switch(channelCount) {
          case 1: {
            return (
              ChannelPlacement::FrontCenter
            );
          }
          case 2: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontRight
            );
          }
          case 3: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontRight |
              ChannelPlacement::FrontCenter
            );
          }
          case 4: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontRight |
              ChannelPlacement::BackLeft |
              ChannelPlacement::BackRight
            );
          }
          case 5: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontCenter |
              ChannelPlacement::FrontRight |
              ChannelPlacement::BackLeft |
              ChannelPlacement::BackRight
            );
          }
          case 6: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontCenter |
              ChannelPlacement::FrontRight |
              ChannelPlacement::BackLeft |
              ChannelPlacement::BackRight |
              ChannelPlacement::LowFrequencyEffects
            );
          }
          case 7: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontCenter |
              ChannelPlacement::FrontRight |
              ChannelPlacement::SideLeft |
              ChannelPlacement::SideRight |
              ChannelPlacement::BackCenter |
              ChannelPlacement::LowFrequencyEffects
            );
          }
          case 8: {
            return (
              ChannelPlacement::FrontLeft |
              ChannelPlacement::FrontCenter |
              ChannelPlacement::FrontRight |
              ChannelPlacement::SideLeft |
              ChannelPlacement::SideRight |
              ChannelPlacement::BackLeft |
              ChannelPlacement::BackRight |
              ChannelPlacement::LowFrequencyEffects
            );
          }
          default: { return ChannelPlacement::Unknown; }
        }
      }
      case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
      case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
      case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE: {
        return ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight;
      }
      default: {
        return ChannelPlacement::Unknown;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement FlacReader::ChannelPlacementFromWaveFormatExtensibleTag(
    const std::string_view &channelMaskValue
  ) {

    // If the channel mask is as least 3 characters long, it's possible that it is specified
    // in hexadecimal format (actually, the specification says that it should be, but since
    // there's still the '0x' prefix and it could be left out, we'll be courteous.
    if(channelMaskValue.length() >= 3) {
      bool isHexadecimal = (
        (channelMaskValue[0] == u8'0') &&
        (
          (channelMaskValue[1] == u8'x') ||
          (channelMaskValue[1] == u8'X')
        )
      );
      if(isHexadecimal) {
        return static_cast<Nuclex::Audio::ChannelPlacement>(
          std::stoul(
            std::string(channelMaskValue.data() + 2, channelMaskValue.length() - 2),
            nullptr,
            16
          )
        );
      }
    }

    // Channel mask is either shorter than 3 characters or doesn't begin with
    // the characters '0x', parse it in decimal format.
    return static_cast<Nuclex::Audio::ChannelPlacement>(
      std::stoul(
        std::string(channelMaskValue.data(), channelMaskValue.length()),
        nullptr,
        10
      )
    );

  }

  // ------------------------------------------------------------------------------------------- //

  FlacReader::FlacReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    state(),
    streamDecoder(Platform::FlacApi::NewStreamDecoder()),
    error(),
    obtainedMetadata(false),
    obtainedChannelMask(false),
    channelAssignment(),
    totalFrameCount(std::uint64_t(-1)),
    trackInfo(nullptr),
    frameCursor(0),
    userPointerForCallback(nullptr),
    processDecodedSamplesCallback(nullptr),
    remainingFrameCount() {

    // We want the VorbisComment block as well because non-standard audio channel
    // sets in FLAC are stored as a WAVEFORMATEXTENSIBLE_CHANNELMAP=0x tag,
    // and of course the title and language tags are relevant to our TrackInfo.
    Platform::FlacApi::SetRespondMetadata(
      streamDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT
    );

    // Open the FLAC file. The stream decoder is created as a blank objects and
    // then initialized via a set of I/O callbacks to access our virtual file.
    this->state = FileAdapterFactory::InitStreamDecoderForReading(
      file, streamDecoder, this
    );

    // The InitStreamDecoderForReading() method will already have checked for errors,
    // but if some file access error happened that libflac deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    FileAdapterState::RethrowPotentialException(*state);

  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::ReadMetadata(TrackInfo &target) {
    this->processDecodedSamplesCallback = nullptr;
    this->remainingFrameCount = 0;

    // Let the stream decoder process FLAC data blocks. They will be delivered via
    // the callbacks set up together with the virtual file I/O callbacks (libflac's design
    // is like that), and with the 'isReadingMetadata' property set, the callbacks will
    // trigger an early stop right after the first audio frame has been decoded.
    this->trackInfo = &target;
    Platform::FlacApi::ProcessUntilEndOfStream(this->streamDecoder);
    this->trackInfo = nullptr;

    // Errors may have happened in either the virtual file (if this happens, it's considered
    // the root cause and other errors are merely follow-up problems from there) or libflac
    // encountered problems decoding the file, in which case we saved the error in order to
    // not throw exceptions right through unsuspecting C code. Both need to be checked.
    FileAdapterState::RethrowPotentialException(*state);
    if(static_cast<bool>(this->error)) {
      std::rethrow_exception(this->error);
    }

    // Finally, no error may have happened, but the FLAC metadata block might not have been
    // encountered. This might be fine for libflac in streaming scenarios, but we absolutely
    // want to know the exact specifications of the audio data we're dealing with.
    if(!this->obtainedMetadata) {
      throw Errors::CorruptedFileError(u8"FLAC audio file is missing the metadata block");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t FlacReader::CountTotalFrames() const {
    assert(this->obtainedMetadata && u8"Frame count must be queried after reading metadata");
    return this->totalFrameCount;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t FlacReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::Seek(std::uint64_t frameIndex) {
    Platform::FlacApi::SeekAbsolute(this->streamDecoder, frameIndex);
    this->frameCursor = frameIndex;
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::DecodeSeparated(
    void *userPointer,
    ProcessDecodedSamplesFunction *processDecodedSamples,
    std::size_t frameCount
  ) {
    this->userPointerForCallback = userPointer;
    this->processDecodedSamplesCallback = processDecodedSamples;
    this->remainingFrameCount = frameCount;

    // Let the stream decoder process FLAC data blocks. They will be delivered via
    // the callbacks set up together with the virtual file I/O callbacks (libflac's design
    // is like that), and with the 'isReadingMetadata' property set, the callbacks will
    // trigger an early stop right after the first audio frame has been decoded.
    this->trackInfo = nullptr;
    Platform::FlacApi::ProcessUntilEndOfStream(this->streamDecoder);

    // Errors may have happened in either the virtual file (if this happens, it's considered
    // the root cause and other errors are merely follow-up problems from there) or libflac
    // encountered problems decoding the file, in which case we saved the error in order to
    // not throw exceptions right through unsuspecting C code. Both need to be checked.
    FileAdapterState::RethrowPotentialException(*state);
    if(static_cast<bool>(this->error)) {
      std::rethrow_exception(this->error);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::ProcessMetadata(
    const ::FLAC__StreamMetadata &metadata
  ) noexcept {
    if(metadata.type == FLAC__METADATA_TYPE_STREAMINFO) {
      processStreamInfo(metadata.data.stream_info);
    } else if(metadata.type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
      processVorbisComment(metadata.data.vorbis_comment);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool FlacReader::ProcessAudioFrame(
    const ::FLAC__Frame &frame,
    const ::FLAC__int32 *const buffers[]
  ) {
    this->channelAssignment = frame.header.channel_assignment;

    if((this->trackInfo != nullptr) && (!this->obtainedChannelMask)) {
      this->trackInfo->ChannelPlacements = (
        ChannelPlacementFromChannelCountAndAssignment(
          this->trackInfo->ChannelCount,
          frame.header.channel_assignment
        )
      );
    }

    // VERIFY: It seems that frame.header.blocksize is the delivered sample count
    std::size_t deliveredFrameCount = frame.header.blocksize;

    // Update the frame cursor (so we know whether seeking is needed) and
    // the number of remaining samples. If no more samples were requested,
    // stop decoding at this point by returning false.
    this->frameCursor += deliveredFrameCount;

    // If we're decoding (rather than just snatching the channel assignment for our metadata),
    // deliver the decoded samples to the callback.
    if(this->processDecodedSamplesCallback != nullptr) {
      this->processDecodedSamplesCallback(
        this->userPointerForCallback, buffers, deliveredFrameCount
      );

      if(deliveredFrameCount < this->remainingFrameCount) {
        this->remainingFrameCount -= deliveredFrameCount;
        return true;
      } else {
        this->remainingFrameCount = 0;
      }
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::HandleError(
    ::FLAC__StreamDecoderErrorStatus status
  ) noexcept {
    if(!static_cast<bool>(this->error)) {
      std::string message(u8"Error decoding FLAC audio file: ", 32);
      message.append(FLAC__StreamDecoderErrorStatusString[status]);
      this->error = std::make_exception_ptr(Errors::CorruptedFileError(message));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::processStreamInfo(
    const ::FLAC__StreamMetadata_StreamInfo &streamInfo
  ) noexcept {
    if(this->trackInfo != nullptr) {

      // Fetch the channel count and use it to determine the layout,
      // unless we already encountered a Vorbis comment with a channel mask in it.
      this->trackInfo->ChannelCount = static_cast<std::size_t>(streamInfo.channels);
      if(!this->obtainedChannelMask) {
        this->trackInfo->ChannelPlacements = (
          ChannelPlacementFromChannelCountAndAssignment(
            this->trackInfo->ChannelCount,
            this->channelAssignment.value_or(FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT)
          )
        );
      }

      // Other useful properties regarding audio resolution and sample rate
      this->trackInfo->SampleRate = static_cast<std::size_t>(streamInfo.sample_rate);
      this->trackInfo->BitsPerSample = static_cast<std::size_t>(streamInfo.bits_per_sample);
      this->trackInfo->SampleFormat = SampleFormatFromBitsPerSample(
        streamInfo.bits_per_sample
      );

      // Given the sample rate and total sample count, we can calculate the total
      // playback duration of the entire audio file
      const std::uint64_t MicrosecondsPerSecond = 1'000'000;
      this->trackInfo->Duration = std::chrono::microseconds(
        streamInfo.total_samples * MicrosecondsPerSecond / streamInfo.sample_rate
      );
    }

    this->totalFrameCount = streamInfo.total_samples;
    this->obtainedMetadata = true;
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacReader::processVorbisComment(
    const ::FLAC__StreamMetadata_VorbisComment &vorbisComment
  ) noexcept {
    using Nuclex::Support::Text::StringHelper;

    // If nobody is interested in this information, we might as well not collect it :)
    if(this->trackInfo == nullptr) {
      return;
    }

    // Iterate over all Vorbis comment tags. These generally are just a string each,
    // but by convention that string is in the format 'key=value' for named properties,
    // such as the file's title or the all-important custom channel mask.
    for(std::size_t index = 0; index < vorbisComment.num_comments; ++index) {
      std::string_view comment(
        reinterpret_cast<char *>(vorbisComment.comments[index].entry),
        vorbisComment.comments[index].length
      );

      // Does this comment look like a 'key=value' assignment?
      std::string_view::size_type assignmentIndex = comment.find(u8'=');
      if(assignmentIndex != std::string_view::npos) {
        std::string_view name = StringHelper::GetTrimmed(comment.substr(0, assignmentIndex));

        // For non-standard channel layouts, FLAC files can contain a channel mask
        // matching the one in Microsoft's Waveform audio file format as a Vorbis comment
        // tag. If this is such a tag, parse the channel layout from it.
        if(name == std::string_view(u8"WAVEFORMATEXTENSIBLE_CHANNEL_MASK")) {
          Nuclex::Audio::ChannelPlacement channelPlacements = (
            FlacReader::ChannelPlacementFromWaveFormatExtensibleTag(
              StringHelper::GetTrimmed(comment.substr(assignmentIndex + 1))
            )
          );
          if(channelPlacements != Nuclex::Audio::ChannelPlacement::Unknown) {
            this->trackInfo->ChannelPlacements = channelPlacements;
            this->obtainedChannelMask = true;
          }
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
