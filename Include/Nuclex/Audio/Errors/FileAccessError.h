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

#ifndef NUCLEX_AUDIO_ERRORS_FILEACCESSERROR_H
#define NUCLEX_AUDIO_ERRORS_FILEACCESSERROR_H

#include "Nuclex/Audio/Config.h"

#include <system_error> // for std::system_error

// DESIGN: Both Nuclex.Pixels + Nuclex.Audio will have a FileAccessError class
//
// It's perfectly reasonable to use both libraries in the same application.
// In Nuclex.Audio, file system access is provided for convenience. Just throwing
// a std::system_error would suck because the user would lose the ability to filter
// exceptions by class (which is the main advantage and why I use exceptions over
// error codes).
//
// Current concept: FileAccessError will only be thrown by the minimal file system
// wrappers in Nuclex.Audio. If someone uses Nuclex.Storage, they are expected to
// write their own glue (it's a matter of <10 lines) that adapts a VirtualFile from
// this library a File from Nuclex.Storage.
//
// Then all file access errors happening in such an application will be
// Nuclex::Storage::Errors::FileAccessError.
//
//
// The alternative, adding a 3rd library with shared exception definitions would add
// additional hoops for users of this library
//

namespace Nuclex { namespace Audio { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a file was not found or could not be opened</summary>
  /// <remarks>
  ///   <para>
  ///     This error will be thrown if anything went wrong accessing the data of a
  ///     virtual file (<see cref="Nuclex.Audio.Storage.VirtualFile" />). If you implement
  ///     your own data sources/sinks using the virtual file interface, all exceptions
  ///     thrown should ideally inherit from this exception
  ///   </para>
  ///   <para>
  ///     If you get this error while working with Nuclex.Audio, it means that your
  ///     image load or save operation has failed not due to a problem with the library,
  ///     but with the underlying stream - a file may be unreadable or your custom
  ///     virtual file implementation will have failed to fetch and transmit data.
  ///   </para>
  /// </remarks>
  class NUCLEX_AUDIO_TYPE FileAccessError : public std::system_error {

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_AUDIO_API explicit FileAccessError(
      std::error_code errorCode, const std::string &message
    ) : std::system_error(errorCode, message) {}

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_AUDIO_API explicit FileAccessError(
      std::error_code errorCode, const char *message
    ) : std::system_error(errorCode, message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Errors

#endif // NUCLEX_AUDIO_ERRORS_FILEACCESSERROR_H
