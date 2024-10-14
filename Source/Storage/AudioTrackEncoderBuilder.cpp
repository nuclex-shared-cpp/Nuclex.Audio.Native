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

#include "Nuclex/Audio/Storage/AudioTrackEncoderBuilder.h"

#include "Nuclex/Audio/Storage/VirtualFile.h"

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &AudioTrackEncoderBuilder::SetStereoChannels() {
    static const std::vector<ChannelPlacement> stereoChannels = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight
    };

    SetChannels(stereoChannels);

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &AudioTrackEncoderBuilder::SetFiveDotOneChannels() {
    static const std::vector<ChannelPlacement> fiveDotOneChannels = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontRight,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::LowFrequencyEffects,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight
    };

    SetChannels(fiveDotOneChannels);

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &AudioTrackEncoderBuilder::SetFiveDotOneChannelsInVorbisOrder() {
    static const std::vector<ChannelPlacement> vorbisFiveDotOneChannels = {
      ChannelPlacement::FrontLeft,
      ChannelPlacement::FrontCenter,
      ChannelPlacement::FrontRight,
      ChannelPlacement::BackLeft,
      ChannelPlacement::BackRight,
      ChannelPlacement::LowFrequencyEffects
    };

    SetChannels(vorbisFiveDotOneChannels);

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackEncoder> AudioTrackEncoderBuilder::Build(
    const std::string &outputFilePath
  ) {
    Build(VirtualFile::OpenRealFileForWriting(outputFilePath));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
