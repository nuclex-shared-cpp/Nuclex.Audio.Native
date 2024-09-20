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

#include "./OpusAudioCodec.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "./OpusDetection.h"
#include "./OpusVirtualFileAdapter.h"
#include "./OpusTrackDecoder.h"
#include "./OpusReader.h"
#include "../../Platform/OpusApi.h"

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Extracts information about a WavPack file into a TrackInfo object</summary>
  /// <param name="opusFile">Opened WavPack audio file the informations are taken from</param>
  /// <param name="trackInfo">Target TrackInfo instance that will be filled</param>
  void extractTrackInfo(
    std::shared_ptr<::OggOpusFile> opusFile, Nuclex::Audio::TrackInfo &trackInfo
  ) {
    using Nuclex::Audio::Platform::OpusApi;
    using Nuclex::Audio::Storage::Opus::OpusReader;

    const ::OpusHead &header = OpusApi::GetHeader(opusFile);

    trackInfo.ChannelCount = static_cast<std::size_t>(header.channel_count);

    trackInfo.ChannelPlacements = OpusReader::ChannelPlacementFromMappingFamilyAndChannelCount(
      header.mapping_family, trackInfo.ChannelCount
    );

    // Opus audio is always encoded at 48000 samples per second, no matter what the original
    // input sample rate had been. The .input_sample_rate field merely states what
    // the original sample rate had been, but is not useful for playback of the Opus file.
    //trackInfo.SampleRate = static_cast<std::size_t>(header.input_sample_rate)
    trackInfo.SampleRate = 48000;

    std::uint64_t totalSampleCount = OpusApi::CountSamples(opusFile);
    trackInfo.Duration = std::chrono::microseconds(totalSampleCount * 1'000 / 48);

    {
      // Completely unfounded, arbitrary value to estimate the precision (which may or may
      // not even change depending on Opus bitrates) of an Opus file compared to any audio
      // format that stores signed integer samples.
      const std::size_t MadeUpOpusPrecisionFromCompressionRatio = 80;

      // Calculate the number of bytes the audio data would decode to
      std::uint64_t decodedByteCount = totalSampleCount * 2; // bytes
      decodedByteCount *= trackInfo.ChannelCount;

      trackInfo.BitsPerSample = std::max<std::size_t>(
        1, // Let's not report less than 1 bit per sample...
        OpusApi::GetRawContainerSize(opusFile) *
        MadeUpOpusPrecisionFromCompressionRatio /
        decodedByteCount
      );
    }

    trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  const std::string &OpusAudioCodec::GetName() const {
    const static std::string codecName(u8"Opus", 4);
    return codecName;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::string> &OpusAudioCodec::GetFileExtensions() const  {
    const static std::vector<std::string> extensions {
      std::string(u8"opus", 4),
      std::string(u8"ogg", 3)
    };

    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> OpusAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // As the AudioCodec interface promises, if the file is not an Opuss audio file,
    // we'll return an empty result to indicate that we couldn't read it.
    if(!Detection::CheckIfOpusHeaderPresent(*source)) {
      return std::optional<ContainerInfo>();
    }

    OpusReader reader(source);

    // WavPack file is now opened, extract the informations the caller requested.
    ContainerInfo containerInfo;
    containerInfo.DefaultTrackIndex = 0;

    // Standalone .wv files only have a single track, always.
    TrackInfo &trackInfo = containerInfo.Tracks.emplace_back();
    reader.ReadMetadata(trackInfo);
    trackInfo.CodecName = GetName();

    return containerInfo;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> OpusAudioCodec::OpenDecoder(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */,
    std::size_t trackIndex /* = 0 */
  ) const {
    (void)extensionHint;
    if(trackIndex != 0) {
      throw std::runtime_error(
        u8"Alternate track decoding is not implemented yet, track index must be 0"
      );
    }

    return std::make_shared<OpusTrackDecoder>(source);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
