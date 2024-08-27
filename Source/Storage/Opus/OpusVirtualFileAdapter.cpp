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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "./OpusVirtualFileAdapter.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <algorithm> // for std::copy_n()

#include <Nuclex/Support/ScopeGuard.h> // for ScopeGuard

#include "Nuclex/Audio/Storage/VirtualFile.h" // for VitualFile

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads up to <see cref="byteCount" /> of data from a virtual file</summary>
  /// <param name="state">State of the virtual file adapter class</param>
  /// <param name="data">Buffer to store the data</param>
  /// <param name="byteCount">Maximum number of bytes to read.</param>
  /// <returns>The numberof bytes successfully read, or a negative value on error</returns>
  /// <remarks>
  ///   The opusfile library allows this function to read fewer bytes than requested,
  ///   except for zero, which is only allowed when the end of the file is reached.
  /// </remarks>
  int opusReadBytes(void *state, std::uint8_t *data, int byteCount) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves the file cursor to a different position within the virtual file</summary>
  /// <param name="state">State of the virtual file adapter class</param>
  /// <param name="offset">Offset relative to the specific anchor point</param>
  /// <param name="anchor">Anchor point, can be SEEK_SET, SEEK_CUR or SEEK_END</param>
  /// <returns>Zero on success, -1 on error or if seeking is not supported</returns>
  int opusSeek(void *state, opus_int64 offset, int anchor) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the current position of the file cursor in the virtual file</summary>
  /// <param name="state">State of the virtual file adapter class</param>
  /// <returns>
  ///   The absolute position of the file cursor or a negative value if seeking is not supported
  // </returns>
  opus_int64 opusTell(void *state) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Closes the virtual file after the opusfile library is done with it</summary>
  /// <param name="state">State of the virtual file adapter class</param>
  /// <returns>Zero on success, EOF if the file could not be closed</returns>
  int opusClose(void *state) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
