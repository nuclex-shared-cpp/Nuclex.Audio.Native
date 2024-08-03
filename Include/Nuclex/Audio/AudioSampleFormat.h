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

#ifndef NUCLEX_AUDIO_AUDIOSAMPLEFORMAT_H
#define NUCLEX_AUDIO_AUDIOSAMPLEFORMAT_H

#include "Nuclex/Audio/Config.h"

#include <string> // for std::string
#include <cstddef> // for std::size_t

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Where audio channels should be played back relative to the viewer<?summary>
  /// <remarks>
  ///   These placement flags are guaranteed to be a superset of the channel mask flags
  ///   Microsoft uses for .wav files, using the same bit masks for all placements shared
  ///   with the .wav format.
  /// </remarks>
  enum class AudioSampleFormat {

    /// <summary>Unknown or unsupported sample format</summary>
    Unknown = 0,

    /// <summary>Welcome to the stone age, unsigned 8-bit audio samples</summary>
    /// <remarks>
    ///   This format uses an offset of 128, so a value of 128 means zero amplitude while
    ///   valid values range symmetrically from 1 to 255 (and 0 is an invalid value).
    ///   Supported by FLAC, this format is rarely found in the wild. It may have found
    ///   some use for audio sensing (i.e. surveillance and (loud) noise detection).
    /// </remarks>
    UnsignedInteger_8 = 8,

    /// <summary>Samples use symmetrical 16-bit integers</summary>
    /// <remarks>
    ///   This format is very widely used. It has been the standard for PCM (uncompressed
    ///   audio found in .wav files, Blu-Rays with lossless DTS-HD MA audio tracks and more).
    ///   Samples are stored as 16-bit integers with a range from -32767 up to 32767.
    ///   The value -32768 goes unused and is invalid but will be interpreted to mean -32767.
    /// </remarks>
    SignedInteger_16 = 16,

    /// <summary>Samples use symmetrical 24-bit integers</summary>
    /// <remarks>
    ///   Some Blu-Ray discs using DTS-HD MA and high definition FLAC music sold in online
    ///   stores is catering to audiophiles using 24-bit audio samples. FLAC added support
    ///   for 24-bit samples in 2002 with version 1.0.4. Samples are stored as signed 24-bit
    ///   integers with a valid range from -8'388'607 to 8'388'607. The extra value,
    ///   -8'388'608, remains unused but will be interpreted to mean -8'388'607.
    /// </remarks>
    SignedInteger_24 = 24,

    /// <summary>Samples use symmetrical 32-bit integers</summary>
    /// <remarks>
    ///   This is a relatively rare format. FLAC added support for it with version 1.4.0,
    ///   released in 2022. It will likely stay a niche format since most audio standards
    ///   stop at 24-bit (consider, for example, USB Audio Class 2.0, used by nearly all
    ///   USB sound cards / audio interfaces, stops at 24-bit, 192 KHz) and since you can
    ///   just switch to floats at 32-bit, which are used during audio production anyway.
    ///   The valid range is -1'073'741'823 to 1"073'741'823 with the negative extra value,
    ///   -1'073'741'824, being unused but interpreted as -1'073'741'823.
    /// </remarks>
    SignedInteger_32 = 32,

#if 0
    /// <summary>Samples use half-precision floats</summary>
    /// <remarks>
    /// </remarks>
    Float_16 = 48,
#endif

    /// <summary>Samples stored as normalized 32-bit floating point values</summary>
    /// <remarks>
    ///   Floating point audio samples are used during audio production due to their
    ///   higher accuracy, which is (due to the more dense distribution of floatiing point
    ///   values near zero) very advantageous to keep quiet passages intact during editing.
    ///   Even &quot;lossless&quot; audio tracks on Blu-Rays, CDs or online stores have
    ///   been converted from their floating point originals to a delivery format that uses
    ///   16-bit or 24-bit integer samples. Most lossy compression formats, especially OPUS
    ///   and AAC, can decompress directly to floating point audio.
    /// </remarks>
    Float32 = 64,

    /// <summary>Samples stored as normalized 64-bit floating point values</summary>
    /// <remarks>
    ///   See the description for 32-bit floating point audio (<see cref="Float32" />).
    ///   Mainly used during audio production, using 64-bit floating point increases
    ///   the precision greatly, which is useful when audio tracks get filtered and adjusted
    ///   many times during the production cycle, to minimize generation loss.
    /// </remarks>
    Float64 = 128

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_AUDIOSAMPLEFORMAT_H
