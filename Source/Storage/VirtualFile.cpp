#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "RealFile.h"

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<const VirtualFile> VirtualFile::OpenRealFileForReading(
    const std::string &path, bool promiseSequentialAccess /* = false */
  ) {
    return std::make_shared<const RealFile>(path, promiseSequentialAccess, true);
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<VirtualFile> VirtualFile::OpenRealFileForWriting(
    const std::string &path, bool promiseSequentialAccess /* = false */
  ) {
    return std::make_shared<RealFile>(path, promiseSequentialAccess, false);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
