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

#include "./FlacAudioCodec.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <stdexcept>

#include <Nuclex/Support/Text/UnicodeHelper.h>
#include <Nuclex/Support/Text/StringHelper.h>

#include "./FlacVirtualFileAdapter.h"
#include "./FlacDetection.h"
#include "./FlacTrackDecoder.h"
#include "./FlacReader.h"
#include "../../Platform/FlacApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decode processor that captures the first stream information block</summary>
  class StreamInfoProcessor : public Nuclex::Audio::Storage::Flac::FlacDecodeProcessor {

    /// <summary>Initializes a new StreamInfo capture decode processor</summary>
    public: StreamInfoProcessor() :
      GotChannelMask(false),
      GotTrackInfo(false),
      ChannelAssignment(FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT),
      TrackInfo() {}

    /// <summary>Frees all memory used by the processor</summary>
    public: virtual ~StreamInfoProcessor() = default;

    /// <summary>Called to process an audio frame after it has been decoded</summary>
    /// <param name="frame">Informations about the decoded audio frame</param>
    /// <param name="buffer">Stores the decoded audio samples</param>
    /// <returns>True to continue decoding, false to stop at this point</returns>
    public: virtual bool ProcessAudioFrame(
      const ::FLAC__Frame *frame,
      const ::FLAC__int32 *const buffer[]
    ) {
      using Nuclex::Audio::Storage::Flac::FlacReader;

      this->ChannelAssignment = frame->header.channel_assignment;
      if(!this->GotChannelMask) {
        this->TrackInfo.ChannelPlacements = (
          FlacReader::ChannelPlacementFromChannelCountAndAssignment(
            this->TrackInfo.ChannelCount,
            this->ChannelAssignment
          )
        );
      }

      return !this->GotTrackInfo; // We're not actually interested in the audio samples
    }

    /// <summary>Called to process any metadata encountered in the FLAC file</summary>
    /// <param name="metadata">Metadata the FLAC stream decoder has encountered</param>
    public: virtual void ProcessMetadata(
      const ::FLAC__StreamMetadata *metadata
    ) noexcept {
      if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        processStreamInfo(metadata->data.stream_info);
      } else if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        processVorbisComment(metadata->data.vorbis_comment);
      }
    }

    /// <summary>Processes a StreamInfo block encountered in the FLAC file</summary>
    /// <param name="streamInfo">StreamInfo block describing the file's properties</param>
    private: void processStreamInfo(const ::FLAC__StreamMetadata_StreamInfo &streamInfo) {
      using Nuclex::Audio::Storage::Flac::FlacReader;

      this->TrackInfo.ChannelCount = static_cast<std::size_t>(streamInfo.channels);
      if(!this->GotChannelMask) {
        this->TrackInfo.ChannelPlacements = (
          FlacReader::ChannelPlacementFromChannelCountAndAssignment(
            static_cast<std::size_t>(streamInfo.channels),
            this->ChannelAssignment // may be filled, may still be defaulted, that's okay
          )
        );
      }

      const std::uint64_t MicrosecondsPerSecond = 1'000'000;
      TrackInfo.Duration = std::chrono::microseconds(
        streamInfo.total_samples * MicrosecondsPerSecond / streamInfo.sample_rate
      );

      this->TrackInfo.SampleRate = static_cast<std::size_t>(streamInfo.sample_rate);
      this->TrackInfo.BitsPerSample = static_cast<std::size_t>(streamInfo.bits_per_sample);

      if(this->TrackInfo.BitsPerSample >= 25) {
        this->TrackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
      } else if(this->TrackInfo.BitsPerSample >= 17) {
        this->TrackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
      } else {
        this->TrackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
      }

      this->GotTrackInfo = true;
    }

    /// <summary>Processes a Vorbis comment block encountered in the FLAC file</summary>
    /// <param name="vorbisComment">Vorbis comment block containing individual entries</param>
    private: void processVorbisComment(
      const ::FLAC__StreamMetadata_VorbisComment &vorbisComment
    ) {
      using Nuclex::Support::Text::StringHelper;
      using Nuclex::Audio::Storage::Flac::FlacReader;

      for(std::size_t index = 0; index < vorbisComment.num_comments; ++index) {
        std::string_view comment(
          reinterpret_cast<char *>(vorbisComment.comments[index].entry),
          vorbisComment.comments[index].length
        );
        std::string_view::size_type assignmentIndex = comment.find(u8'=');
        if(assignmentIndex != std::string_view::npos) {
          std::string_view name = StringHelper::GetTrimmed(comment.substr(0, assignmentIndex));
          if(name == std::string_view(u8"WAVEFORMATEXTENSIBLE_CHANNEL_MASK")) {
            Nuclex::Audio::ChannelPlacement channelPlacements = (
              FlacReader::ChannelPlacementFromWaveFormatExtensibleTag(
                StringHelper::GetTrimmed(comment.substr(assignmentIndex + 1))
              )
            );
            if(channelPlacements != Nuclex::Audio::ChannelPlacement::Unknown) {
              this->TrackInfo.ChannelPlacements = channelPlacements;
              this->GotChannelMask = true;
            }
          }
        }
      }
    }

    /// <summary>Called to provide a detailed status when a decoding error occurs</summary>
    /// <param name="status">Error status of the stream decoder</param>
    public: virtual void HandleError(
      ::FLAC__StreamDecoderErrorStatus status
    ) noexcept {}

    /// <summary>Whether a WaveFormatExtensible channel mask was encountered</summary>
    public: bool GotChannelMask;
    /// <summary>Whether the TrackInfo field was filled with data</summary>
    public: bool GotTrackInfo;
    /// <summary>Channel assignment set that is used by the FLAC file</summary>
    public: ::FLAC__ChannelAssignment ChannelAssignment;
    /// <summary>TrackInfo instance that receives the audio file informations</summary>
    public: Nuclex::Audio::TrackInfo TrackInfo;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  const std::string &FlacAudioCodec::GetName() const {
    const static std::string codecName(u8"FLAC", 4);
    return codecName;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::string> &FlacAudioCodec::GetFileExtensions() const  {
    const static std::vector<std::string> extensions {
      std::string(u8"flac", 4),
      std::string(u8"ogg", 3)
    };

    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> FlacAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // As the AudioCodec interface promises, if the file is not an Opuss audio file,
    // we'll return an empty result to indicate that we couldn't read it.
    if(!Detection::CheckIfFlacHeaderPresent(*source)) {
      return std::optional<ContainerInfo>();
    }

    StreamInfoProcessor processor;

    // Explicit scope for the FLAC stream decoder to ensure it is destroyed together
    // with the virtual file adapter state.
    {
      std::shared_ptr<::FLAC__StreamDecoder> streamDecoder = (
        Platform::FlacApi::NewStreamDecoder()
      );

      // We want the VorbisComment block as well because non-standard audio channel
      // sets in FLAC are stored as a WAVEFORMATEXTENSIBLE_CHANNELMAP=0x tag.
      Platform::FlacApi::SetRespondMetadata(
        streamDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT
      );

      // Open the FLAC file. The stream decoder is created as a blank objects and
      // then initialized via a set of I/O callbacks to access our virtual file.
      std::unique_ptr<ReadOnlyFileAdapterState> state = (
        FileAdapterFactory::InitStreamDecoderForReading(source, streamDecoder, &processor)
      );

      // The InitStreamDecoderForReading() method will already have checked for errors,
      // but if some file access error happened that libflac deemed non-fatal,
      // we still want to throw it - an exception in VirtualFile should always surface.
      FileAdapterState::RethrowPotentialException(*state);

      ::FLAC__bool result = (
        ::FLAC__stream_decoder_process_until_end_of_stream(streamDecoder.get())
      );

      // The earlier call to RethrowPotentialException() is unlikely to ever have any
      // exceptions as the current implementation inside libflac only begins accessing
      // the file when told to decode it.
      FileAdapterState::RethrowPotentialException(*state);

    } // streamDecoder closing scope (so it's destroyed earlier than the virtual file adapter)

    if(processor.GotTrackInfo) {
      ContainerInfo containerInfo;
      containerInfo.DefaultTrackIndex = 0;

      // Standalone .flac files only have a single track, always.
      processor.TrackInfo.CodecName = GetName();
      containerInfo.Tracks.push_back(processor.TrackInfo);

      return containerInfo;
    } else {
      throw std::runtime_error(u8"Could not find stream info block in FLAC stream");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> FlacAudioCodec::OpenDecoder(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */,
    std::size_t trackIndex /* = 0 */
  ) const {
    if(trackIndex != 0) {
      throw std::runtime_error(
        u8"Alternate track decoding is not implemented yet, track index must be 0"
      );
    }

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
