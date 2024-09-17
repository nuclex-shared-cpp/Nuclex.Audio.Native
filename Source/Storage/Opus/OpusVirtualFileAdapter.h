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

#ifndef NUCLEX_AUDIO_STORAGE_OPUS_OPUSVIRTUALFILEADAPTER_H
#define NUCLEX_AUDIO_STORAGE_OPUS_OPUSVIRTUALFILEADAPTER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "../Shared/VirtualFileAdapterState.h"

#include <memory> // for std::unique_ptr

#include <opusfile.h> // for op

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream adapters</summary>
  struct FileAdapterState : public Shared::VirtualFileAdapterState {};

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream reader adapter</summary>
  struct ReadOnlyFileAdapterState : public FileAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<const VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the Opus stream writer adapter</summary>
  struct WritableFileAdapterState : public FileAdapterState {

    /// <summary>Virtual file this adapter is forwarding calls to</summary>
    public: std::shared_ptr<VirtualFile> File;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Constructs Opus file adapters and hooks up file callbacks</summary>
  class FileAdapterFactory {

    /// <summayr>Constructs a virtual file adapter for a read-only file</summary>
    /// <param name="readOnlyFile">Virtual file the adapter will read from</param>
    /// <param name="fileCallbacks">
    ///   Opus file callback set that will be set up to use the adapter
    /// </param>
    /// <returns>
    ///   A state that needs to be passed as the 'state' parameter though libopusfile
    /// </returns>
    public: static std::unique_ptr<ReadOnlyFileAdapterState> CreateAdapterForReading(
      const std::shared_ptr<const VirtualFile> &readOnlyFile,
      ::OpusFileCallbacks &fileCallbacks
    );

    /// <summayr>Constructs a virtual file adapter for a writable file</summary>
    /// <param name="writableFile">Virtual file the adapter will write to</param>
    /// <param name="fileCallbacks">
    ///   Opus file callback set that will be set up to use the adapter
    /// </param>
    /// <returns>
    ///   A state that needs to be passed as the 'state' parameter though libopusfile
    /// </returns>
    public: static std::unique_ptr<WritableFileAdapterState> CreateAdapterForWriting(
      const std::shared_ptr<VirtualFile> &writableFile,
      ::OpusFileCallbacks &fileCallbacks
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)

#endif // NUCLEX_AUDIO_STORAGE_OPUS_OPUSVIRTUALFILEADAPTER_H
