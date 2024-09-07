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

#include "./WavPackAudioCodec.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/Storage/VirtualFile.h"

#include "./WavPackDetection.h"
#include "./WavPackVirtualFileAdapter.h"
#include "./WavPackTrackDecoder.h"

#include "../../Platform/WavPackApi.h" // for WavPackApi

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Extracts information about a WavPack file into a TrackInfo object</summary>
  /// <param name="context">Opened WavPack audio file the informations are taken from</param>
  /// <param name="trackInfo">Target TrackInfo instance that will be filled</param>
  void extractTrackInfo(
    std::shared_ptr<::WavpackContext> context, Nuclex::Audio::TrackInfo &trackInfo
  ) {
    using Nuclex::Audio::Platform::WavPackApi;
    using Nuclex::Audio::Storage::WavPack::WavPackTrackDecoder;

    trackInfo.ChannelCount = static_cast<std::size_t>(WavPackApi::GetNumChannels(context));

    // We can just cast this one to our enumeration. Why? The ChannelPlacement enumeration
    // uses the same values that the Microsoft Waveform audio file format also uses, which
    // happens to be what WavPack uses, too, thus WavPack's channel masks are equivalent.
    trackInfo.ChannelPlacements = static_cast<Nuclex::Audio::ChannelPlacement>(
      WavPackApi::GetChannelMask(context)
    );

    // This returns the number of "complete samples" (aka frames), meaning the number
    // samples per channel (rather than the sum of the sample counts in all channels).
    std::uint64_t frameCount = WavPackApi::GetNumSamples64(context);

    std::uint32_t sampleRate = WavPackApi::GetSampleRate(context);
    trackInfo.SampleRate = static_cast<std::size_t>(sampleRate);
    
    // We want an accurate result (some audio sync tool or such may depend on it),
    // so we multiply first. No bounds checking under the assumption that nobody will
    // ever feed this library a WavPack file with more than 584'942 years of audio.        
    const std::uint64_t MicrosecondsPerSecond = 1'000'000;
    trackInfo.Duration = std::chrono::microseconds(
      frameCount * MicrosecondsPerSecond / sampleRate
    );

    trackInfo.SampleFormat = WavPackTrackDecoder::SampleFormatFromModeAndBitsPerSample(
      WavPackApi::GetMode(context), WavPackApi::GetBitsPerSample(context)
    );
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  const std::string &WavPackAudioCodec::GetName() const {
    const static std::string codecName(u8"WavPack", 7);
    return codecName;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::string> &WavPackAudioCodec::GetFileExtensions() const {
    const static std::vector<std::string> extensions { std::string(u8"wv", 2) };
    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> WavPackAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // As the AudioCodec interface promises, if the file is not a WavPack audio file,
    // we'll return an empty result to indicate that we couldn't read it. All other
    // errors happen after we decided that it's a WavPack file, so from then onwards
    // the errors are due to a corrupt file or other and will cause exceptions.
    if(!Detection::CheckIfWavPackHeaderPresent(*source)) {
      return std::optional<ContainerInfo>();
    }

    // Set up a WavPack stream reader with adapter methods that will perform all reads
    // on the user-provided virtual file.
    ::WavpackStreamReader64 streamReader;
    std::unique_ptr<ReadOnlyStreamAdapterState> state = (
      StreamAdapterFactory::CreateAdapterForReading(source, streamReader)
    );

    // Explicit scope for the WavPackContext to ensure it is destroyed before
    // the virtual file adapter gets killed (in case libwavpack wants to fetch
    // additional data from the file while we examine it).
    {

      // Open the WavPack file, obtaining a WavPack context. Everything inside
      // this scope is just error plumbing code, ensuring that the right exception
      // surfaces if either libwavpack reports an error or the virtual file throws.
      std::shared_ptr<::WavpackContext> context = Platform::WavPackApi::OpenStreamReaderInput(
        state->Error, // exception_ptr that will receive VirtualFile exceptions
        streamReader,
        state.get() // passed to all IO callbacks as void pointer
      );

      // The OpenStreamReaderInput() method will already have checked for errors,
      // but if some file access error happened that libwavpack deemed non-fatal,
      // we still want to throw it - an exception in VirtualFile should always surface.
      StreamAdapterState::RethrowPotentialException(*state);

      // WavPack file is now opened, extract the informations the caller requested.
      ContainerInfo containerInfo;
      containerInfo.DefaultTrackIndex = 0;

      // Standalone .wv files only have a single track, always.
      TrackInfo &trackInfo = containerInfo.Tracks.emplace_back();
      trackInfo.CodecName = GetName();
      extractTrackInfo(context, trackInfo);

      return containerInfo;

    } // WavPack context closing scope (so it's destroyed earlier than the virtual file adapter)
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> WavPackAudioCodec::OpenDecoder(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */,
    std::size_t trackIndex /* = 0 */
  ) const {
    if(trackIndex != 0) {
      throw std::runtime_error(
        u8"Alternate track decoding is not implemented yet, track index must be 0"
      );
    }

    // The constructor will throw if the file cannot be opened by libwavpack.
    return std::make_shared<WavPackTrackDecoder>(source);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
