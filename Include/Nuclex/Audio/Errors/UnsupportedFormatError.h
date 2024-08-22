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

#ifndef NUCLEX_AUDIO_ERRORS_UNSUPPORTEDFORMATERROR_H
#define NUCLEX_AUDIO_ERRORS_UNSUPPORTEDFORMATERROR_H

#include "Nuclex/Audio/Config.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Audio { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a file has an unsupported format or an unsupported format sub-type
  /// </summary>
  class NUCLEX_AUDIO_TYPE UnsupportedFormatError : public std::runtime_error {

    /// <summary>Initializes a new unsupported format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_AUDIO_API explicit UnsupportedFormatError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new unsupported format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_AUDIO_API explicit UnsupportedFormatError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Errors

#endif // NUCLEX_AUDIO_ERRORS_UNSUPPORTEDFORMATERROR_H
