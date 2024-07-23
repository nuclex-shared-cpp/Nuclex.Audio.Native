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

#ifndef NUCLEX_AUDIO_TRACK_H
#define NUCLEX_AUDIO_TRACK_H

#include "Nuclex/Audio/Config.h"

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Audio track containing a variable number of channels</summary>
  class NUCLEX_AUDIO_TYPE Track {

    // Plan
    //
    // The basic idea is to have a container to store audio data and become
    // independent of any audio codecs and external channel placement flags.
    //
    // Codecs should be registerable (like in Nuclex.Pixels) so support for
    // invividual codecs can be toggled on and off.
    //
    // Tracks should either be autonomous (meaning they hold all of the audio
    // data in their own buffer) or remain connected to a decoder that allows
    // them to stream additional audio as needed.
    //
    // Encoding should happen through a separate encoder (so unlike BitmapSerializer,
    // the two sides will be split here). The encoder should have a progress
    // notification and support cancellation.


    // Streaming Design:
    //
    // - Having the channels pull data from the AudioStreamer or such?
    //   * AudioStreamer needs to buffer data anyway because it is interleaved
    //   * But channels could fetch data wide that's wide apart
    // - Having the track update the channels manually?
    //   * Ensures all tracks always have the same section of samples
    //     * Could even hand-off to user: EnsureDecodedRangeAvailable(begin, end)
    //   * But would be spooky if channels had their state change through track?
    //     * Maybe a complete non-issue with proper design?

    // Type safety
    //
    // - It is neat to have audio tracks and channels templated (see
    //   AudioFile<T> from https://github.com/adamstark/AudioFile)
    // - But applied to such a basic level of the library, complicates things
    //
    // - Just Channel<T> inheriting Channel?
    //   * Via method Channel<T> Channel::Aa<T>?
    //   * Could multiple typed wrappers exist for the same channel?
    // - Channel has inherent type, either defined at load time or changeable?
    //   * No extra bookkeeping and conversion for Channel<T>
    //   * But if changeable, might invalidate existing Channel<T>

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_TRACK_H
