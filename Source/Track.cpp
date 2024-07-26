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

#include "Nuclex/Audio/Track.h"

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  void Track::SetName(
    const std::optional<std::string> &newName /* = std::optional<std::string>() */
  ) {
    this->name = newName;
  }

  // ------------------------------------------------------------------------------------------- //

  void Track::SetLanguageCode(
    const std::optional<std::string> &newLanguageCode /* = std::optional<std::string>() */
  ) {
    this->languageCode = newLanguageCode;
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio
