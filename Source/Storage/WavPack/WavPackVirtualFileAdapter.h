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
  struct StreamAdapterState {

    /// <summary>
    ///   Re-throws any exceptions that happened while libwavpack accessed the adapter
    /// </summary>
    /// <param name="streamAdapterState">State that will be checked for an exception</param>
    public: static void RethrowPotentialException(StreamAdapterState &state);

    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsReadOnly;
    /// <summary>Current position of the emulated file cursor</summary>
    public: std::uint64_t FileCursor;
    /// <summary>Bytes that have been buffered for read operations</summary>
    public: std::vector<std::uint8_t> BufferedBytes;
    /// <summary>Stores any exception thrown by the virtual file interface</summary>
    public: std::exception_ptr Error;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream reader adapter</summary>
  struct ReadOnlyStreamAdapterState : public StreamAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<const VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the WavPack stream writer adapter</summary>
  struct WritableStreamAdapterState : public StreamAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Constructs WavPack StreamReader adapters and hooks up stream readers</summary>
  class StreamAdapterFactory {

    /// <summayr>Constructs a StreamReader adapter for a read-only stream</summary>
    /// <param name="readOnlyFile">Virtual file the adapter will read from</param>
    /// <param name="streamReader">
    ///   WavPack StreamReader that will be set up to use the adapter
    /// </param>
    /// <returns>
    ///   A state that needs to be passed as the 'id' parameter though WavPack
    /// </returns>
    public: static std::unique_ptr<ReadOnlyStreamAdapterState> CreateAdapterForReading(
      const std::shared_ptr<const VirtualFile> &readOnlyFile,
      ::WavpackStreamReader64 &streamReader
    );

    /// <summayr>Constructs a StreamReader adapter for a writable stream</summary>
    /// <param name="writableFile">Virtual file the adapter will write to</param>
    /// <param name="streamReader">
    ///   WavPack StreamReader that will be set up to use the adapter
    /// </param>
    /// <returns>
    ///   A state that needs to be passed as the 'id' parameter though WavPack
    /// </returns>
    public: static std::unique_ptr<WritableStreamAdapterState> CreateAdapterForWriting(
      const std::shared_ptr<VirtualFile> &writableFile,
      ::WavpackStreamReader64 &streamReader
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKVIRTUALFILEADAPTER_H
