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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/AudioSampleFormat.h"

#include <memory> // for std::unique_ptr, std::shared_ptr

#include <wavpack.h> // for the WavPack types

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  struct ReadOnlyStreamAdapterState;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility class with intermediate methods used to decode FLAC files</summary>
  class WavPackReader {

    /// <summary>Determines the native sample format from WavPack's parameters</summary>
    /// <param name="mode">Encoder mode that WavPack file was encoded with</param>
    /// <param name="bitsPerSample">The number of valid bits in each sample</param>
    /// <returns>The equivalent sample format enumeration value</returns>
    public: static AudioSampleFormat SampleFormatFromModeAndBitsPerSample(
      int mode, int bitsPerSample
    );

    /// <summary>Initializes a new WavPack reader accessing the specified file</summary>
    /// <param name="file">File that will be opened by the reader</param>
    public: WavPackReader(const std::shared_ptr<const VirtualFile> &file);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~WavPackReader();

    /// <summary>File the reader is accessing</summary>
    private: std::shared_ptr<const VirtualFile> file;
    /// <summary>
    ///   Stores callbacks through which libwavpack accesses the VirtualFile instance
    /// </summary>
    /// <remarks>
    ///   When opening the WavPack file, a pointer to this structure is stored Within
    ///   the WavPackContext, so it needs to continue to exist.
    /// </remarks>
    private: ::WavpackStreamReader64 streamReader;
    /// <summary>Holds the state of the VirtualFile stream adapter</summary>
    /// <remarks>
    ///   This stores the emulated file cursor for the specific WavPack context and 
    ///   records any exceptions from the VirtualFile so they do not have to pass through
    ///   libwavpack, which is C code and won't anticipate being interrupted this way.
    /// </remarks>
    private: std::unique_ptr<ReadOnlyStreamAdapterState> state;
    /// <summary>Represents the opened WavPack file for the libwavpack API</summary>
    /// <remarks>
    ///   This ties all the internal WavPack data structure together and is passed
    ///   to any public API method in libwavpack we can call.
    /// </remarks>
    private: std::shared_ptr<::WavpackContext> context;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKREADER_H
