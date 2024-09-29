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

#ifndef NUCLEX_AUDIO_PLATFORM_WINDOWSFILEAPI_H
#define NUCLEX_AUDIO_PLATFORM_WINDOWSFILEAPI_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_WINDOWS)

#include "WindowsApi.h"

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps file access functions from the Windows file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is just a small helper class that reduces the amount of boilerplate code
  ///     required when calling the file system API functions from Windows, such as
  ///     checking result codes and transforming paths from UTF-8 to UTF-16 stored in
  ///     wchar_ts of non-standard 2 byte size.
  ///   </para>
  ///   <para>
  ///     It is not intended to hide operating system details or make this API platform
  ///     neutral (the File and Container classes do that), so depending on the amount
  ///     of noise required by the file system APIs, only some methods will be wrapped here.
  ///   </para>
  /// </remarks>
  class WindowsFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="sequentialAccess">Whether the file will be read sequentially</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForReading(const std::string &path, bool sequentialAccess);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="sequentialAccess">Whether the file will be written sequentially</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForWriting(const std::string &path, bool sequentialAccess);

    /// <summary>Returns the total size of the file in bytes</summary>
    /// <param name="fileHandle">Handle of the file whose size will be reported</param>
    /// <returns>The size of the file with the specified handle in bytes</returns>
    public: static std::uint64_t GetFileSize(::HANDLE fileHandle);

    /// <summary>Moves the file cursor to a different position</summary>
    /// <param name="fileHandle">Handle of the file whose file cursor to move</param>
    /// <param name="offset">Offset to move the file cursor relative to the anchor</param>
    /// <param name="anchor">Anchor relative to which the file cursor will be placed</param>
    /// <returns>The new absolute position of the file cursor from the file start</returns>
    public: static std::size_t Seek(HANDLE fileHandle, std::ptrdiff_t offset, DWORD anchor);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="fileHandle">Handle of the file from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(
      HANDLE fileHandle, std::byte *buffer, std::size_t count
    );

    /// <summary>Writes data into the specified file</summary>
    /// <param name="fileHandle">Handle of the file into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(
      HANDLE fileHandle, const std::byte *buffer, std::size_t count
    );

    /// <summary>Closes the specified file</summary>
    /// <param name="fileHandle">Handle of the file that will be closed</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void CloseFile(HANDLE fileHandle, bool throwOnError = true);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorCode">
    ///   Value that GetLastError() returned at the time of failure
    /// </param>
    public: [[noreturn]] static void ThrowExceptionForFileAccessError(
      const std::string &errorMessage, DWORD errorCode
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_WINDOWS)

#endif // NUCLEX_AUDIO_PLATFORM_WINDOWSFILEAPI_H
