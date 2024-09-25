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

#ifndef NUCLEX_AUDIO_TRACKINFO_H
#define NUCLEX_AUDIO_TRACKINFO_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/AudioSampleFormat.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include <cstddef> // for std::size_t
#include <optional> // for std::optional
#include <string> // for std::string
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Informations about an audio track (containing one or more channels)</summary>
  /// <remarks>
  ///   This structure is returned if you ask a codec to provide informations about
  ///   an audio file before actually loading it.
  /// </remarks>
  struct NUCLEX_AUDIO_TYPE TrackInfo {

    // ----------------------------------------------------------------------------------------- //

    // From the module that decoded it

    /// <summary>Name of the codec used to compress / store the audio samples</summary>
    public: std::string CodecName;

    // ----------------------------------------------------------------------------------------- //

    // From the media container (.mpa, .mka, .ogg) usually. Often empty.

    /// <summary>The name of the audio track, if provided by the container</summary>
    /// <remarks>
    ///   For single track containers such as .wav, .flac or .opus, there will only be
    ///   a track name if the file uses music tagging. For multi-track containers such
    ///   as .mka, there is a wealth of information about each audio track, usually
    ///   also including a human-readable name. Use to aid in selection by the user,
    ///   but anticipate that this string may be missing.
    /// </remarks>
    public: std::optional<std::string> Name;

    /// <summary>The language of the audio track in rfc-5646 format</summary>
    /// <remarks>
    ///   The rfc-5646 format defines language tags as you may have seen in mkv
    ///   files and other uses on the internet. They look like this: en-us,
    ///   en-uk or de-de (or just de).
    /// </remarks>
    public: std::optional<std::string> LanguageCode;

    // ----------------------------------------------------------------------------------------- //

    // From the actual audio data

    /// <summary>Number of audio channels in the track</summary>
    public: std::size_t ChannelCount;

    /// <summary>Placements for which this track provides audio channels</summary>
    public: ChannelPlacement ChannelPlacements;

    /// <summary>Duration of the audio track</summary>
    public: std::chrono::microseconds Duration;

    /// <summary>Samples per second in each channel, typically 44100 or 48000</summary>
    public: std::size_t SampleRate;

    /// <summary>Format in which the audio samples are recorded</summary>
    /// <remarks>
    ///   <para>
    ///     This can, but doesn't have to be the same format in which you read data.
    ///     For offline audio processing, you could simply always request the data as
    ///     fldeal with floats or
    ///     doub
    ///     For example, to read 24-bit audio you will have to use arrays of
    ///     32-bit integers (which will be appropriately repeat-padded). You can use
    ///   this to decide on a code path, i.e. hav
    /// </remarks>
    public: AudioSampleFormat SampleFormat;

    /// <summary>Number of bits actually used for a sample</summary>
    /// <remarks>
    ///   <para>
    ///     To figure out the exposed audio sample format, check <see cref="SampleFormat" />.
    ///   </para>
    ///   <para>
    ///     This is only for informative purposes. Some audio formats can have arbitrary
    ///     bit depths that are not multiples of 8. The Nuclex.Audio.Native library will
    ///     handle that for you (via bit-repeat-padding or conversion), but you can see
    ///     the true bit depth of the encoded audio data using this field.
    ///   </para>
    /// </remarks>
    public: std::size_t BitsPerSample;

    // ----------------------------------------------------------------------------------------- //

    // Helpers

    // CHECK: Is IsMono() confusing? If one splits 5.1 audio into 6 channels they would
    //        be mono, but this method would return false because they're not
    //        placed in the front center position.
    // -> IsClassicMono() ?
    // -> IsSimpleMono() ?

    /// <summary>Whether the audio track is a normal mono track</summary>
    /// <remarks>
    ///   This is a simple conveniency method. It will check if there is exactly one
    ///   audio channel and that channel is intended for the front center speaker.
    /// </remarks>
    public: NUCLEX_AUDIO_API bool IsMono() const;

    /// <summary>Whether the audio track is a normal stereo track</summary>
    /// <remarks>
    ///   This is a simple conveniency method. It will check if there are exactly two
    ///   audio channels and those channel are intended for the front left and right
    ///   speakers.
    /// </remarks>
    public: NUCLEX_AUDIO_API bool IsStereo() const;

    /// <summary>Whether the audio track is a typical 5.1 surround track</summary>
    /// <remarks>
    ///   This is a simple conveniency method. It will check if there are exactly six
    ///   audio channels consisting of 5 surround channels and one low frequency effect
    ///   channel. There are two 5.1 surround formats, one with the rearmore channels
    ///   being back left/right and one with them being side left/right. This checks
    ///   for the former only!
    /// </remarks>
    public: NUCLEX_AUDIO_API bool IsFiveDotOne() const;

    /// <summary>Whether the audio track is a 5.1 surround (side) track</summary>
    /// <remarks>
    ///   This is a simple conveniency method. It will check if there are exactly six
    ///   audio channels consisting of 5 surround channels and one low frequency effect
    ///   channel. There are two 5.1 surround formats, one with the rearmore channels
    ///   being back left/right and one with them being side left/right. This checks
    ///   for the latter only!
    /// </remarks>
    public: NUCLEX_AUDIO_API bool IsFiveDotOneSide() const;

    /// <summary>Whether the audio track is a 7.1 surround track</summary>
    /// <remarks>
    ///   This is a simple conveniency method. It will check if there are exactly eight
    ///   audio channels consisting of 7 surround channels and one low frequency effect
    ///   channel. This format is often used on UHD Blu-Rays and can easily be downmixed
    ///   to 5.1 by merging the side left/right and back left/right channels.
    /// </remarks>
    public: NUCLEX_AUDIO_API bool IsSevenDotOne() const;

  };

  // ------------------------------------------------------------------------------------------- //

  inline bool TrackInfo::IsMono() const {
    const ChannelPlacement monoPlacement = (
      ChannelPlacement::FrontCenter
    );

    return (this->ChannelCount == 1) && (this->ChannelPlacements == monoPlacement);
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool TrackInfo::IsStereo() const {
    const ChannelPlacement stereoPlacement = (
      ChannelPlacement::FrontLeft | ChannelPlacement::FrontRight
    );

    return (this->ChannelCount == 2) && (this->ChannelPlacements == stereoPlacement);
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool TrackInfo::IsFiveDotOne() const {
    const ChannelPlacement fiveDotOnePlacement = (
      ChannelPlacement::FrontLeft |
      ChannelPlacement::FrontCenter |
      ChannelPlacement::FrontRight |
      ChannelPlacement::BackLeft |
      ChannelPlacement::BackRight |
      ChannelPlacement::LowFrequencyEffects
    );

    return (this->ChannelCount == 6) && (this->ChannelPlacements == fiveDotOnePlacement);
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool TrackInfo::IsFiveDotOneSide() const {
    const ChannelPlacement fiveDotOneSidePlacement = (
      ChannelPlacement::FrontLeft |
      ChannelPlacement::FrontCenter |
      ChannelPlacement::FrontRight |
      ChannelPlacement::SideLeft |
      ChannelPlacement::SideRight |
      ChannelPlacement::LowFrequencyEffects
    );

    return (this->ChannelCount == 6) && (this->ChannelPlacements == fiveDotOneSidePlacement);
  }

  // ------------------------------------------------------------------------------------------- //

  inline bool TrackInfo::IsSevenDotOne() const {
    const ChannelPlacement sevenDotOnePlacement = (
      ChannelPlacement::FrontLeft |
      ChannelPlacement::FrontCenter |
      ChannelPlacement::FrontRight |
      ChannelPlacement::SideLeft |
      ChannelPlacement::SideRight |
      ChannelPlacement::BackLeft |
      ChannelPlacement::BackRight |
      ChannelPlacement::LowFrequencyEffects
    );

    return (this->ChannelCount == 8) && (this->ChannelPlacements == sevenDotOnePlacement);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_TRACKINFO_H
