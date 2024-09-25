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

#ifndef NUCLEX_AUDIO_STORAGE_SHARED_VIRTUALFILEADAPTERSTATE_H
#define NUCLEX_AUDIO_STORAGE_SHARED_VIRTUALFILEADAPTERSTATE_H

#include "Nuclex/Audio/Config.h"

#include <cstdint> // for std::uint64_t
#include <exception> // for std::exception_ptr

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Shared {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores informations processed by the audio stream adapters</summary>
  struct VirtualFileAdapterState {

    /// <summary>
    ///   Re-throws any exceptions that happened while libopsufile accessed the adapter
    /// </summary>
    /// <param name="streamAdapterState">State that will be checked for an exception</param>
    public: static void RethrowPotentialException(VirtualFileAdapterState &state);

    /// <summary>Whether this environment supports writing to the virtual file</summary>
    public: bool IsReadOnly;
    /// <summary>Current position of the emulated file cursor</summary>
    public: std::uint64_t FileCursor;
    /// <summary>Stores any exception thrown by the virtual file interface</summary>
    public: std::exception_ptr Error;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Shared

#endif // NUCLEX_AUDIO_STORAGE_SHARED_VIRTUALFILEADAPTERSTATE_H
