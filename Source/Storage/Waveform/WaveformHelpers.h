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

#ifndef NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMHELPERS_H
#define NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMHELPERS_H

#include "Nuclex/Audio/Config.h"

#include "Nuclex/Audio/ChannelPlacement.h" // for ChannelPlacement

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class with utility functions for reading Waveform files</summary>
  class Helpers {

    /// <summary>Guesses the most likely channel placement given a channel count</summary>
    /// <param name="channelCount">Number of channels in the Waveform audio file</param>
    /// <returns>The likely placement of the channels</returns>
    public: static ChannelPlacement GuessChannelPlacement(std::size_t channelCount);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform

#endif // NUCLEX_AUDIO_STORAGE_WAVEFORM_WAVEFORMHELPERS_H
