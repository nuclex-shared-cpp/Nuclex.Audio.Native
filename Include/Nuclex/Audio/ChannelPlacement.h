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
  ///   The first 
  /// </remarks>
  enum class ChannelPlacement {

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
    ///     In a 5.1 or higher setup, this channel should play 30 degrees to the front left of
    ///     the direction the listener is facing in.
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
    ///     In a 5.1 or higher setup, this channel should play 30 degrees to the front right of
    ///     the direction the listener is facing in.
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

    BackLeft = 16, // matches SPEAKER_BACK_LEFT in WAVE files
    BackRight = 32, // matches SPEAKER_BACK_RIGHT in WAVE files
    FrontCenterLeft = 64, // matches SPEAKER_FRONT_LEFT_OF_CENTER in WAVE files
    FrontCenterRight = 128, // matches SPEAKER_FRONT_RIGHT_OF_CENTER in WAVE files
    BackCenter = 256, // matches SPEAKER_BACK_CENTER in WAVE files
    SideLeft = 512, // matches SPEAKER_SIDE_LEFT in WAVE files
    SideRight = 1024, // matches SPEAKER_SIDE_RIGHT in WAVE files
    TopCenter = 2048, // matches SPEAKER_TOP_CENTER in WAVE files
    TopFrontLeft = 4096, // matches SPEAKER_TOP_FRONT_LEFT in WAVE files
    TopFrontCenter = 8192, // matches SPEAKER_TOP_FRONT_CENTER in WAVE files
    TopFrontRight = 16384, // matches SPEAKER_TOP_FRONT_RIGHT in WAVE files
    TopBackLeft = 32768, // matches SPEAKER_TOP_BACK_LEFT in WAVE files
    TopBackCenter = 65536, // matches SPEAKER_TOP_BACK_CENTER in WAVE files
    TopBackRight = 131072 // matches SPEAKER_TOP_BACK_RIGHT in WAVE files

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_OPUSTRANSCODER_AUDIO_CHANNELPLACEMENT_H
