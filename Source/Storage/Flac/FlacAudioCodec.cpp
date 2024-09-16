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

    ContainerInfo containerInfo;
    containerInfo.DefaultTrackIndex = 0;
    {
      FlacReader reader(source);
      reader.ReadMetadata(containerInfo.Tracks.emplace_back());
    }

    return containerInfo;
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

    //return std::make_shared<FlacTrackDecoder>(source);
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
