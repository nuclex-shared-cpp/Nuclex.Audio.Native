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
#include "../../Platform/OpusApi.h"

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks a virtual file / Opus callback adapter for exceptions</summary>
  /// <remarks>
  ///   <para>
  ///     We can't throw exceptions right through libopusfile's C code (well, we could,
  ///     but it would interrupt execution at points where libopusfile doesn't expect it),
  ///     so the virtual file adapter records exceptions that come out of the virtual file
  ///     being accessed.
  ///   </para>
  ///   <para>
  ///     This small helper class will check if there are any exceptions recorded in
  ///     a virtual file adapter and re-throw them. It is used by the OpusApi methods
  ///     to re-throw the root cause exception rather than the generic libopusfile error
  ///     if the original failure occurred in the virtual file class.
  ///   </para>
  /// </remarks>
  class ExceptionChecker {

    /// <summary>Initializes a new exception checker for the specified adapter state</summary>
    /// <param name="state">Adapter state that will be checked</param>
    public: ExceptionChecker(Nuclex::Audio::Storage::Opus::FileAdapterState &state) :
      state(state) {}

    /// <summary>
    ///   Checks whether any exceptions have occurred in the file adapter and, if so,
    ///   re-throws them
    /// </summary>
    public: void Check() {
      Nuclex::Audio::Storage::Opus::FileAdapterState::RethrowPotentialException(
        this->state
      );
    }

    /// <summary>The stream adapter from which exceptions will be re-thrown</summary>
    private: Nuclex::Audio::Storage::Opus::FileAdapterState &state;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  const std::string &OpusAudioCodec::GetName() const {
    const static std::string codecName(u8"Opus", 18);
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

    // Set up the libopusfile callbacks with adapter methods that will perform all reads
    // on the user-provided virtual file.
    ::OpusFileCallbacks fileCallbacks;
    std::unique_ptr<ReadOnlyFileAdapterState> state = (
      FileAdapterFactory::CreateAdapterForReading(source, fileCallbacks)
    );

    // Explicit scope for the OggOpusFile to ensure it is destroyed before
    // the virtual file adapter gets killed (in case libopusfile wants to fetch
    // additional data from the file while we examine it).
    {

      // Open the Opus file, obtaining a OggOpusFile instance. Everything inside
      // this scope is just error plumbing code, ensuring that the right exception
      // surfaces if either libopusfile reports an error or the virtual file throws.
      std::shared_ptr<::OggOpusFile> opusFile;
      {
        using Nuclex::Audio::Platform::OpusApi;

        ExceptionChecker exceptionChecker(*state);
        opusFile = OpusApi::OpenFromCallbacks(
          Nuclex::Support::Events::Delegate<void()>::Create<
            ExceptionChecker, &ExceptionChecker::Check
          >(&exceptionChecker),
          state.get(),
          &fileCallbacks
        );

        // The OpenFromCallbacks() method will already have checked for errors,
        // but if some file access error happened that libopusfile deemed non-fatal,
        // we still want to throw it - an exception in VirtualFile should always surface.
        FileAdapterState::RethrowPotentialException(*state);
      }

      // OGG/Opus file is now opened, extract the informations the caller requested.
      ContainerInfo containerInfo;
      containerInfo.DefaultTrackIndex = 0;

      // Standalone .opus files only have a single track, always.
      TrackInfo &trackInfo = containerInfo.Tracks.emplace_back();
      trackInfo.CodecName = GetName();
      //extractTrackInfo(context, trackInfo);

      return containerInfo;

    } // opusFile closing scope (so it's destroyed earlier than the virtual file adapter)
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
