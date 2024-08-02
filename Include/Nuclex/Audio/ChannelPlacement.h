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

#ifndef NUCLEX_AUDIO_CHANNELPLACEMENT_H
#define NUCLEX_AUDIO_CHANNELPLACEMENT_H

#include "Nuclex/Audio/Config.h"

#include <string> // for std::string
#include <cstddef> // for std::size_t

// Reference 1:
// There are three generations of Microsoft's well-known .wav audio format, using different
// headers: WAVEFORMAT, WAVEFORMATEX and WAVEFORMATEXTENSIBLE. The lattermost one includes
// a set of channel mask flags that have become a bit of a standard in audio channel orders.
#if defined(DONT_ENABLE_MICROSOFT_WAVE_CHANNELS_FOR_REFERENCE_ONLY)
  SPEAKER_FRONT_LEFT = 0x1,
  SPEAKER_FRONT_RIGHT = 0x2,
  SPEAKER_FRONT_CENTER = 0x4,
  SPEAKER_LOW_FREQUENCY = 0x8,
  SPEAKER_BACK_LEFT = 0x10,
  SPEAKER_BACK_RIGHT = 0x20,
  SPEAKER_FRONT_LEFT_OF_CENTER = 0x40,
  SPEAKER_FRONT_RIGHT_OF_CENTER = 0x80,
  SPEAKER_BACK_CENTER = 0x100,
  SPEAKER_SIDE_LEFT = 0x200,
  SPEAKER_SIDE_RIGHT = 0x400,
  SPEAKER_TOP_CENTER = 0x800,
  SPEAKER_TOP_FRONT_LEFT = 0x1000,
  SPEAKER_TOP_FRONT_CENTER = 0x2000,
  SPEAKER_TOP_FRONT_RIGHT = 0x4000,
  SPEAKER_TOP_BACK_LEFT = 0x8000,
  SPEAKER_TOP_BACK_CENTER = 0x10000,
  SPEAKER_TOP_BACK_RIGHT = 0x20000,
  SPEAKER_RESERVED = 0x80000000
#endif

// Reference 2:
// A relatively popular library for loading audio files, libsndfile, has established its own
// channel map flags that don't match the .wav flags. The library's design treats even most
// error codes as "prone to change," so these values may not hold much meaning outside of
// an application using sndfile.h with a statically linked libsndfile.
#if defined(DONT_ENABLE_LIBSNDFILE_CHANNEL_MAPS_FOR_REFERENCE_ONLY)
  SF_CHANNEL_MAP_INVALID = 0,
  SF_CHANNEL_MAP_MONO = 1,
  SF_CHANNEL_MAP_LEFT,
  SF_CHANNEL_MAP_RIGHT,
  SF_CHANNEL_MAP_CENTER,
  SF_CHANNEL_MAP_FRONT_LEFT,
  SF_CHANNEL_MAP_FRONT_RIGHT,
  SF_CHANNEL_MAP_FRONT_CENTER,
  SF_CHANNEL_MAP_REAR_CENTER,
  SF_CHANNEL_MAP_REAR_LEFT,
  SF_CHANNEL_MAP_REAR_RIGHT,
  SF_CHANNEL_MAP_LFE,
  SF_CHANNEL_MAP_FRONT_LEFT_OF_CENTER,
  SF_CHANNEL_MAP_FRONT_RIGHT_OF_CENTER,
  SF_CHANNEL_MAP_SIDE_LEFT,
  SF_CHANNEL_MAP_SIDE_RIGHT,
  SF_CHANNEL_MAP_TOP_CENTER,
  SF_CHANNEL_MAP_TOP_FRONT_LEFT,
  SF_CHANNEL_MAP_TOP_FRONT_RIGHT,
  SF_CHANNEL_MAP_TOP_FRONT_CENTER,
  SF_CHANNEL_MAP_TOP_REAR_LEFT,
  SF_CHANNEL_MAP_TOP_REAR_RIGHT,
  SF_CHANNEL_MAP_TOP_REAR_CENTER,
  SF_CHANNEL_MAP_AMBISONIC_B_W,
  SF_CHANNEL_MAP_AMBISONIC_B_X,
  SF_CHANNEL_MAP_AMBISONIC_B_Y,
  SF_CHANNEL_MAP_AMBISONIC_B_Z,
#endif

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Where audio channels should be played back relative to the viewer<?summary>
  /// <remarks>
  ///   These placement flags are guaranteed to be a superset of the channel mask flags
  ///   Microsoft uses for .wav files, using the same bit masks for all placements shared
  ///   with the .wav format.
  /// </remarks>
  enum class ChannelPlacement : std::size_t {

    /// <summary>Placement not determined yet or wasn't specified</summary>
    Unknown = 0,

    /// <summary>Front left corner speaker or generic left side stereo speaker</summary>
    /// <remarks>
    ///   <para>
    ///     In a basic stereo setup, this would either be a speaker on the left side of
    ///     the room, the TV screen or the left headphone speaker.
    ///   </para>
    ///   <para>
    ///     Note that dedicated headphone mixes aren't a thing - rather, if the software or
    ///     audio equipment knows headphones are being used, the stereo separation is decreased
    ///     a bit (by mixing a percentage of the opposite channel in).
    ///   </para>
    ///   <para>
    ///     In a 5.1 or higher setup, this channel should play around 22.5 to 30 degrees
    ///     to the front left of the direction the listener is facing in.
    ///   </para>
    /// </remarks>
    FrontLeft = 1, // matches SPEAKER_FRONT_LEFT in WAVE files

    /// <summary>Front right corner speaker or generic right side stereo speaker</summary>
    /// <remarks>
    ///   <para>
    ///     In a basic stereo setup, this would either be a speaker on the right side of
    ///     the room, the TV screen or the right headphone speaker.
    ///   </para>
    ///   <para>
    ///     Note that dedicated headphone mixes aren't a thing - rather, if the software or
    ///     audio equipment knows headphones are being used, the stereo separation is decreased
    ///     a bit (by mixing a percentage of the opposite channel in).
    ///   </para>
    ///   <para>
    ///     In a 5.1 or higher setup, this channel should play around 22.5 to 30 degrees
    ///     to the front right of the direction the listener is facing in.
    ///   </para>
    /// </remarks>
    FrontRight = 2, // matches SPEAKER_FRONT_RIGHT in WAVE files

    /// <summary>Channel should be placed directly in front of the listener</summary>
    FrontCenter = 4, // matches SPEAKER_FRONT_CENTER in WAVE files

    /// <summary>Bass / subwoofer audio coming from every direction</summary>
    /// <remarks>
    ///   Technically, the low frequency speaker is placed somewhere in the room, but it's
    ///   supposed to create sound pressure and body-shaking effects that human ears are 
    ///   not good at locating in space.
    /// </remarks>
    LowFrequencyEffects = 8, // matches SPEAKER_LOW_FREQUENCY in WAVE files

    /// <summary>Channel should be played to the rear left of the listener</summary>
    /// <remarks>
    ///   <para>
    ///     In a 5.1 surround setup, this is just the left side speaker. It should be
    ///     placed between 100 and 120 degrees to the listener's left side. Some audio
    ///     formats differentiate between 5.1 and 5.1(side), this would be the former.
    ///   </para>
    ///   <para>
    ///     In a 7.1 surround setup, this is the true rear left speaker and it should
    ///     be placed farther in the rear, around 130 to 150 deghrees to the left.
    ///   </para>
    /// </remarks>
    BackLeft = 16, // matches SPEAKER_BACK_LEFT in WAVE files

    /// <summary>Channel should be played to the rear right of the listener</summary>
    /// <remarks>
    ///   <para>
    ///     In a 5.1 surround setup, this is just the right side speaker. It should be
    ///     placed between 100 and 120 degrees to the listener's right side. Some audio
    ///     formats differentiate between 5.1 and 5.1(side), this would be the former.
    ///   </para>
    ///   <para>
    ///     In a 7.1 surround setup, this is the true rear right speaker and it should
    ///     be placed farther in the rear, around 130 to 150 deghrees to the right.
    ///   </para>
    /// </remarks>
    BackRight = 32, // matches SPEAKER_BACK_RIGHT in WAVE files

    /// <summary>Channel should be placed between center and front left</summary>
    FrontCenterLeft = 64, // matches SPEAKER_FRONT_LEFT_OF_CENTER in WAVE files

    /// <summary>Channel should be placed between center and front right</summary>
    FrontCenterRight = 128, // matches SPEAKER_FRONT_RIGHT_OF_CENTER in WAVE files

    /// <summary>Channel should be placed behind the listener</summary>
    BackCenter = 256, // matches SPEAKER_BACK_CENTER in WAVE files

    /// <summary>Channel should be played to the right side of the listener</summary>
    /// <remarks>
    ///   <para>
    ///     In both a 5.1 or 7.1 surround setup, this is the left side speaker.
    ///   </para>
    ///   <para>
    ///     For a 5.1 system using side speakers (rather the rear side speakers),
    ///     this channel should be played around 100 to 120 degrees to the left
    ///     of the listener.
    ///   </para>
    ///   <para>
    ///     In a 7.1 system where the rear left/right speakers are further back,
    ///     this speaker can be placed in a range from 60 to 100 degrees to
    ///     the left side of the listener.
    ///   </para>
    /// </remarks>
    SideLeft = 512, // matches SPEAKER_SIDE_LEFT in WAVE files

    /// <summary>Channel should be played to the right side of the listener</summary>
    /// <remarks>
    ///   <para>
    ///     In both a 5.1 or 7.1 surround setup, this is the right side speaker.
    ///   </para>
    ///   <para>
    ///     For a 5.1 system using side speakers (rather the rear side speakers),
    ///     this channel should be played around 100 to 120 degrees to the right
    ///     of the listener.
    ///   </para>
    ///   <para>
    ///     In a 7.1 system where the rear left/right speakers are further back,
    ///     this speaker can be placed in a range from 60 to 100 degrees to
    ///     the right side of the listener.
    ///   </para>
    /// </remarks>
    SideRight = 1024, // matches SPEAKER_SIDE_RIGHT in WAVE files

    /// <summary>Channel should be played from above the listener</summary>
    TopCenter = 2048, // matches SPEAKER_TOP_CENTER in WAVE files

    /// <summary>Speaker is at an elevated placement on the front left</summary>
    /// <remarks>
    ///   This is found in an alternative 7.1 setup that doesn't seem to be too common.
    /// </remarks>
    TopFrontLeft = 4096, // matches SPEAKER_TOP_FRONT_LEFT in WAVE files

    /// <summary>Speaker that is in front and above the listener</summary>
    TopFrontCenter = 8192, // matches SPEAKER_TOP_FRONT_CENTER in WAVE files

    /// <summary>Speaker is at an elevated placement on the front right</summary>
    /// <remarks>
    ///   This is found in an alternative 7.1 setup that doesn't seem to be too common.
    /// </remarks>
    TopFrontRight = 16384, // matches SPEAKER_TOP_FRONT_RIGHT in WAVE files

    /// <summary>Speaker is at an elevated placement on the back left</summary>
    TopBackLeft = 32768, // matches SPEAKER_TOP_BACK_LEFT in WAVE files

    /// <summary>Speaker that is in the rear and above the listener</summary>
    TopBackCenter = 65536, // matches SPEAKER_TOP_BACK_CENTER in WAVE files

    /// <summary>Speaker is at an elevated placement on the back right</summary>
    TopBackRight = 131072 // matches SPEAKER_TOP_BACK_RIGHT in WAVE files

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the bitwise or of two channel placement flags, combining them</summary>
  /// <param name="first">First channel placement flag that will be combined</param>
  /// <param name="second">Second channel placement flag that will be combined</param>
  /// <returns>The channel placements flags combined via a bitwise or</returns>
  inline ChannelPlacement operator |(ChannelPlacement first, ChannelPlacement second) {
    return static_cast<ChannelPlacement>(
      static_cast<std::size_t>(first) | static_cast<std::size_t>(second)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the bitwise and of two channel placement flags, limiting them</summary>
  /// <param name="first">First channel placement flag that will be limited</param>
  /// <param name="second">Second channel placement flag that will be limited</param>
  /// <returns>The channel placements flags limited via a bitwise and</returns>
  inline ChannelPlacement operator &(ChannelPlacement first, ChannelPlacement second) {
    return static_cast<ChannelPlacement>(
      static_cast<std::size_t>(first) & static_cast<std::size_t>(second)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates a string describing the specified channel placement</summary>
  /// <param name="placement">Channel placement for which a string will be generated</param>
  /// <returns>A string containing the specified channel placement as text</returns>
  /// <remarks>
  ///  This is a small helper. You can use it for debugging, to output channel placements
  ///  in log files, error messages or perhaps even to the user in command line programs.
  /// </remarks>
  std::string NUCLEX_AUDIO_API StringFromChannelPlacement(ChannelPlacement placement);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Parses a string generated by <see cref="StringFromChannelPlacement" /> and returns
  ///   the channel placement contained in it
  /// </summary>
  /// <param name="channelPlacementAsText">
  ///   String describing channel placements as generated by
  ///   <see cref="StringFromChannelPlacement" />.
  /// </param>
  /// <returns>The channel placement flags parsed from the string</returns>
  ChannelPlacement NUCLEX_AUDIO_API ChannelPlacementFromString(
    const std::string &channelPlacementAsText
  );

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_CHANNELPLACEMENT_H
