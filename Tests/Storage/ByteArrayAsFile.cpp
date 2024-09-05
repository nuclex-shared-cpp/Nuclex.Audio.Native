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

#include "./ByteArrayAsFile.h"

#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  void ByteArrayAsFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
  ) const {
    assert((start < this->length) && u8"Read starts within file boundaries");
    assert((this->length >= start + byteCount) && u8"Read ends within file boundaries");
    std::copy_n(this->data + start, byteCount, buffer);
  }

  // ------------------------------------------------------------------------------------------- //

  void ByteArrayAsFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
  ) {
    (void)start;
    (void)byteCount;
    (void)buffer;
    assert(!u8"Write method of unit test dummy file is never called");
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
