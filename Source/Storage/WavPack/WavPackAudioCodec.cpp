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

#include "../../Platform/WavPackApi.h" // for WavPackApi

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks a virtual file to WavPack stream adapter for exceptions</summary>
  /// <remarks>
  ///   <para>
  ///     We can't throw exceptions right through libwavpack's C code (well, we could,
  ///     but it would interrupt execution at points where libwavpack doesn't expect it),
  ///     so the virtual file adapter records exceptions that come out of the virtual file
  ///     being accessed.
  ///   </para>
  ///   <para>
  ///     This small helper class will check if there are any exceptions recorded in
  ///     a virtual file adapter and re-throw them. It is used by the LibWavPackApi methods
  ///     to re-throw the root cause exception rather than the generic libwavpack error
  ///     if the original failure occurred in the virtual file class.
  ///   </para>
  /// </remarks>
  class ExceptionChecker {

    /// <summary>Initializes a new exception checker for the specified adapter state</summary>
    /// <param name="state">Adapter state that will be checked</param>
    public: ExceptionChecker(Nuclex::Audio::Storage::WavPack::StreamAdapterState &state) :
      state(state) {}

    /// <summary>
    ///   Checks whether any exceptions have occurred in the stream adapter and, if so,
    ///   re-throws them
    /// </summary>
    public: void Check() {
      Nuclex::Audio::Storage::WavPack::StreamAdapterState::RethrowPotentialException(
        this->state
      );
    }

    /// <summary>The stream adapter from which exceptions will be re-thrown</summary>
    private: Nuclex::Audio::Storage::WavPack::StreamAdapterState &state;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Extracts information about a WavPack file into a TrackInfo object</summary>
  /// <param name="context">Opened WavPack audio file the informations are taken from</param>
  /// <param name="trackInfo">Target TrackInfo instance that will be filled</param>
  void extractTrackInfo(
    std::shared_ptr<::WavpackContext> context, Nuclex::Audio::TrackInfo &trackInfo
  ) {
    using Nuclex::Audio::Platform::WavPackApi;

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
    
    // We want an accurate result (some audio sync tool or such may depend on it),
    // so we multiply first. No bounds checking under the assumption that nobody will
    // ever feed this library a WavPack file with more than 584'942 years of audio.        
    const std::uint64_t MicrosecondsPerSecond = 1'000'000;
    trackInfo.Duration = std::chrono::microseconds(
      frameCount * MicrosecondsPerSecond / WavPackApi::GetSampleRate(context)
    );

    // Figure out the data format closest to the data stored by WavPack. Normally it
    // should be an exact match, but WavPack leaves room to store fewer bits, not only
    // for 24-bit formats. For the sake of robustness, we'll anticipate those, too.
    int mode = WavPackApi::GetMode(context);
    int bitsPerSample = WavPackApi::GetBitsPerSample(context);
    if((mode & MODE_FLOAT) != 0) {
      if(bitsPerSample >= 33) {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_64;
      } else {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::Float_32;
      }
    } else {
      if(bitsPerSample >= 25) {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
      } else if(bitsPerSample >= 17) {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
      } else if(bitsPerSample >= 9) {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
      } else {
        trackInfo.SampleFormat = Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
      }
    }
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
      std::shared_ptr<::WavpackContext> context; 
      {
        using Nuclex::Audio::Platform::WavPackApi;

        ExceptionChecker exceptionChecker(*state);
        context = WavPackApi::OpenStreamReaderInput(
          Nuclex::Support::Events::Delegate<void()>::Create<
            ExceptionChecker, &ExceptionChecker::Check
          >(&exceptionChecker),
          streamReader,
          state.get()
        );

        // The OpenStreamReaderInput() method will already have checked for errors,
        // but if some file access error happened that libwavpack deemed non-fatal,
        // we still want to throw it - an exception in VirtualFile should always surface.
        StreamAdapterState::RethrowPotentialException(*state);
      }

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

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
