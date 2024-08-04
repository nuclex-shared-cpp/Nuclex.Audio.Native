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

#ifndef NUCLEX_AUDIO_STORAGE_WAVE_WAVEHELPERS_H
#define NUCLEX_AUDIO_STORAGE_WAVE_WAVEHELPERS_H

#include "Nuclex/Audio/Config.h"

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Wave {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid PNG file possible</summary>
  /// <remarks>
  ///   From https://github.com/mathiasbynens/small/blob/master/wav.wav
  /// </remarks>
  constexpr const std::size_t SmallestPossibleWaveSize = 44; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading PNG files using libpng</summary>
  class Helpers {

    /// <summary>Checks if the specified file extension indicates a .wav file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates a .wav</returns>
    public: static bool DoesFileExtensionSayWav(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid .wav header</summary>
    /// <param name="source">File that will be checked for a valid .wav header</param>
    /// <returns>True if a valid .wav header was found, false otherwise</returns>
    public: static bool CheckIfWaveHeaderPresent(const VirtualFile &source);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Wave

#endif // NUCLEX_AUDIO_STORAGE_WAVE_WAVEHELPERS_H
