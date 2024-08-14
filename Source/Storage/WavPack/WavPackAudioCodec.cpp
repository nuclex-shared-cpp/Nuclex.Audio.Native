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
#include "./WavPackVirtualFileAdapter.h"
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
    const std::vector<std::string> extensions { std::string(u8"wv", 2) };
    return extensions;
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<ContainerInfo> WavPackAudioCodec::TryReadInfo(
    const std::shared_ptr<const VirtualFile> &source,
    const std::string &extensionHint /* = std::string() */
  ) const {
    (void)extensionHint;

    ::WavpackStreamReader64 streamReader;

    std::unique_ptr<ReadOnlyStreamAdapterState> state = (
      StreamAdapterFactory::CreateAdapterForReading(source, streamReader)
    );

    {
      using Nuclex::Audio::Platform::WavPackApi;

      std::shared_ptr<::WavpackContext> context = (
        WavPackApi::OpenStreamReaderInput(streamReader, state.get())
      );
      
      // Just for testing:
      std::size_t channelCount = WavPackApi::GetNumChannels(context);
    }
    
    return std::optional<ContainerInfo>();
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
