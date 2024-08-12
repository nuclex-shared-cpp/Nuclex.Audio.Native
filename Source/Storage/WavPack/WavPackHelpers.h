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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKHELPERS_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKHELPERS_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t
#include <vector> // for std::vector

#include <wavpack.h> // for all wavpack functions

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid WavPack file possible</summary>
  /// <remarks>
  ///   Created a 2-sample .wav in Audacity, compressed with -h -x via WavPack 5.7
  /// </remarks>
  constexpr const std::size_t SmallestPossibleWavPackSize = 118; // bytes

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading WavPack files</summary>
  class Helpers {

    /// <summary>Checks if the specified file extension indicates a .wv file</summary>
    /// <param name="extension">File extension (can be with or without leading dot)</param>
    /// <returns>True if the file extension indicates a .wv file</returns>
    public: static bool DoesFileExtensionSayWv(const std::string &extension);

    /// <summary>Checks if the specified file starts with a valid WavPack header</summary>
    /// <param name="source">File that will be checked for a valid WavPack header</param>
    /// <returns>True if a valid WavPack header was found, false otherwise</returns>
    public: static bool CheckIfWavPackHeaderPresent(const VirtualFile &source);

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream reader adapter</summary>
  struct WavpackVirtualFileReader {
  
    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsWritable;
    /// <summary>Current position of the emulated file cursor</summary>
    private: std::uint64_t fileCursor;
    /// <summary>Total size of the file in bytes or -1 if not yet determined</summary>
    private: std::uint64_t fileSize;
    /// <summary>Bytes that have been buffered for read operations</summary>
    private: std::vector<std::uint8_t> bufferedBytes;

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    private: std::shared_ptr<const VirtualFile> virtualFile;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream writer adapter</summary>
  struct WavpackWriteEnvironment {

    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsWritable;
    /// <summary>Current position of the emulated file cursor</summary>
    private: std::uint64_t fileCursor;
    /// <summary>Total size of the file in bytes or -1 if not yet determined</summary>
    private: std::uint64_t fileSize;
    /// <summary>Bytes that have been buffered for read operations</summary>
    private: std::vector<std::uint8_t> bufferedBytes;

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    private: std::shared_ptr<VirtualFile> virtualFile;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKHELPERS_H
