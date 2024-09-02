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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr

//
// This started out as 'AudioStreamDecoder' but is now 'AudioTrackDecoder'.
//
// The design is strongly tending towards random access decoding because all
// audio formats I checked have a concept of individually decodable blocks,
// pages or chunks.
//

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes audio of all channels in one audio track</summary>
  class NUCLEX_AUDIO_TYPE AudioTrackDecoder {

    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~AudioTrackDecoder() = default;

    /// <summary>Counts the number of audio channels in the track</summary>
    /// <returns>The number of audio channels in the audio track</returns>
    public: virtual std::size_t CountChannels() const = 0;

    /// <summary>Retrieves the order in which channels are interleaved</summary>
    /// <returns>A list of single channels in the order they're interleaved</returns>
    /// <remarks>
    ///   <para>
    ///     Audio data is generally encoded in an interleaved format, meaning that instead of
    ///     storing the whole of each channel one after another, one audio sample of each
    ///     channel is stored round-robin.
    ///   </para>
    ///   <para>
    ///     That means that for a 3 channel audio file (let's assume left, center, right),
    ///     the audio data would store sample 1: left, center, right, then
    ///     sample 2: left, center, right, then sample 3 and so on.
    ///   </para>
    ///   <para>
    ///     This is needed so that there's no seeking involved with playback, but it also
    ///     means that you need to pay attention to the ordering of channels in the decoded
    ///     audio data if you want to do anything useful with them. This method gives you
    ///     the audio channels in the order in which they are interleaved in the audio data.
    ///   </para>
    /// </remarks>
    public: virtual const std::vector<ChannelPlacement> &GetChannelOrder() const = 0;

    // CountChannels()
    //
    // GetChannelOrder()  <--  Important now
    //
    // GetChannelsPresent()

    // GetBlockSize()
    //
    //   Would return the size of the codec's blocks (all codecs are splitting audio into
    //   individually decodable packets because otherwise, playback with seking would be
    //   impossible).
    //
    //   This could be merely informative (and requesting a "page in" or however it's called
    //   would round outwards to the next block boundaries).
    //
    //   Waveform would have a block size of 1 sample.

    // PageIn(startSample, sampleCount)  ??  Barf -> statefulness, threading issues...
    //
    //   Would make the specified region available for direct reading. Probably a bad
    //   idea since it would prevent usage of the stream decoder from multiple threads.
    //
    //   However, if decoding any block in a random access fashion is the main or only
    //   task this class performs, it would be more like a RandomAccessDecoder :)
    //

    // DecodeInto<sampleType>(...)
    //
    //   Methods could be templated for automatic conversion to the sample type.
    //   A bit icky for 24-bit formats.
    //     - One-off method to decode into 24-bits?
    //     - Generally expose the concept of 'meaningful bits' and 'padding bits'
    //       to the user and do 8/16/32-bit int, float and double only?

    // DecodeInto(sampleContainer, startSample, endSample)
    //
    //   The sampleContainer could be an interface, so the caller could either provide
    //   our default implementation of it (which stores samples and perhaps automatically
    //   adjust indicates so that 0 is the requested sample even if the page/block
    //   had to begin earlier).
    //
    //   For optimal performance, the caller could provide their own implementation of
    //   the interface which lets the decoder write the decoded audio data directly into
    //   any buffer the caller wishes to use.
    //

    // Clone()
    //
    //   Because some codec implementations (Opus as an example) support fast and easy
    //   cloning on the decoder state. But does it make sense? Can blocks/pages be so
    //   large that you'd want to keep a clone around rather than decode it in one go?
    //

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKDECODER_H
