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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKVIRTUALFILEADAPTER_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKVIRTUALFILEADAPTER_H

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

  /// <summary>Stores informations processed by the WavPack stream adapters</summary>
  struct SharedEnvironment {

    /// <summary>Initializes the properties of the shared environment base class</summary>
    /// <param name="readOnly">True if the environment is a pure read environment</param>
    public: SharedEnvironment(bool readOnly = true) :
      IsReadOnly(readOnly),
      FileCursor(0),
      FileSize(std::uint64_t(-1)),
      BufferedBytes(),
      Error() {}

    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsReadOnly;
    /// <summary>Current position of the emulated file cursor</summary>
    public: std::uint64_t FileCursor;
    /// <summary>Total size of the file in bytes or -1 if not yet determined</summary>
    public: std::uint64_t FileSize;
    /// <summary>Bytes that have been buffered for read operations</summary>
    public: std::vector<std::uint8_t> BufferedBytes;
    /// <summary>Stores any exception thrown by the virtual file interface</summary>
    public: std::exception_ptr Error;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream reader adapter</summary>
  struct ReadEnvironment : public SharedEnvironment {

    /// <summary>Initializes a new WavPack read environment</summary>
    /// <param name="streamReader">WavPack stream reader initialized for writing</param>
    /// <param name="file">File from which libwavpack should be reading</param>
    public: ReadEnvironment(
      ::WavpackStreamReader64 &streamReader,
      const std::shared_ptr<VirtualFile> &file
    ) :
      SharedEnvironment(true),
      File(file) {
      SetupFunctionPointers(*this, streamReader);
    }

    /// <summary>Sets up the function pointers used by libwavpack</summary>
    /// <param name="readEnvironment">
    ///   Environment on which the function pointers will be set up
    /// </param>
    /// <param name="streamReader">Main PNG structure initialized for reading</param>
    protected: static void SetupFunctionPointers(
      ReadEnvironment &readEnvironment, ::WavpackStreamReader64 &streamReader
    );

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<const VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream writer adapter</summary>
  struct WriteEnvironment : public SharedEnvironment {

    /// <summary>Initializes a new WavPack write environment</summary>
    /// <param name="streamReader">WavPack stream reader initialized for writing</param>
    /// <param name="file">File from which libwavpack should be reading</param>
    public: WriteEnvironment(
      ::WavpackStreamReader64 &streamReader,
      const std::shared_ptr<VirtualFile> &file
    ) :
      SharedEnvironment(false),
      File(file) {
      //SetupFunctionPointers(*this, pngRead);
    }

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKVIRTUALFILEADAPTER_H
