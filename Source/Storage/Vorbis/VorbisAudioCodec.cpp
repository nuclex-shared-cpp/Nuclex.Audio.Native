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

#include "./VorbisAudioCodec.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "./VorbisDetection.h"
#include "./VorbisVirtualFileAdapter.h"
#include "./VorbisTrackDecoder.h"
#include "./VorbisReader.h"
#include "../../Platform/VorbisApi.h"

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  const std::string &VorbisAudioCodec::GetName() const {
    const static std::string codecName(u8"Vorbis", 6);
    return codecName;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::string> &VorbisAudioCodec::GetFileExtensions() const  {
    const static std::vector<std::string> extensions {
      std::string(u8"ogg", 3)
    };

    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> VorbisAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    // As the AudioCodec interface promises, if the file is not an Vorbiss audio file,
    // we'll return an empty result to indicate that we couldn't read it.
    if(!Detection::CheckIfVorbisHeaderPresentLite(*source)) {
      return std::optional<ContainerInfo>();
    }

    VorbisReader reader(source);

    // WavPack file is now opened, extract the informations the caller requested.
    ContainerInfo containerInfo;
    containerInfo.DefaultTrackIndex = 0;

    // For now, we just read the first track's metadata, assuming that we're
    // dealing with a single audio track encoded to Ogg Vorbis
    TrackInfo &trackInfo = containerInfo.Tracks.emplace_back();
    reader.ReadMetadata(trackInfo);
    trackInfo.CodecName = GetName();

    return containerInfo;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackDecoder> VorbisAudioCodec::TryOpenDecoder(
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

    return std::make_shared<VorbisTrackDecoder>(source);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
