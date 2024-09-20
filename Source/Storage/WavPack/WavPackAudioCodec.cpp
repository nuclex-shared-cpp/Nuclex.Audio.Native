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
#include "./WavPackReader.h"

#include "../../Platform/WavPackApi.h" // for WavPackApi

namespace {

  // ------------------------------------------------------------------------------------------- //
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

    WavPackReader reader(source);

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

  std::shared_ptr<AudioTrackDecoder> WavPackAudioCodec::TryOpenDecoder(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */,
    std::size_t trackIndex /* = 0 */
  ) const {
    (void)extensionHint;

    // As the AudioCodec interface promises, if the file is not an Opus audio file,
    // we'll return an empty result to indicate that we couldn't read it.
    if(!Detection::CheckIfWavPackHeaderPresent(*source)) {
      return std::shared_ptr<AudioTrackDecoder>();
    }

    if(trackIndex != 0) {
      throw std::runtime_error(
        u8"Alternate track decoding is not implemented yet, track index must be 0"
      );
    }

    // The constructor will throw if the file cannot be opened by libwavpack.
    return std::make_shared<WavPackTrackDecoder>(source);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
