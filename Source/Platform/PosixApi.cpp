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

#include "PosixApi.h"

#if !defined(NUCLEX_AUDIO_WINDOWS)

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <vector> // for std::vector
#include <cstring> // string.h for strerror()
#include <system_error> // for std::system_error
#include <cerrno> // for direct access to 'errno'

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function" // defined but not used
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function" // defined but not used
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Overload for the Posix version of strerror_r()</summary>
  /// <param name="buffer">Buffer into which the error message has been written</param>
  /// <returns>A pointer to the start of the error message string</returns>
  const char *getStringFromStrErrorR(int, std::string &buffer) {
    return buffer.c_str();
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Overload for the GNU/Linux version of strerror_r()</summary>
  /// <param name="strErrorReturn">Result returned from the strerror_r() function</param>
  /// <returns>A pointer to the start of the error message string</returns>
  const char *getStringFromStrErrorR(const char *strErrorReturn, std::string &) {
    return strErrorReturn;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_AUDIO_STRERROR_IS_THREAD_LOCAL)
  std::string PosixApi::GetErrorMessage(int errorNumber) {
    errno = 0;
    const char *errorMessageString = ::strerror(errorNumber);

    int errorNumberFromStrError = errno;
    if(errorNumberFromStrError != 0) {
      std::string errorMessage(u8"Error ");
      Nuclex::Support::Text::lexical_append(errorMessage, errorNumber);
      errorMessage.append(u8" (and error message lookup failed)");
      return errorMessage;
    }

    return std::string(errorMessageString);
  }
#else // if we can't be sure strerror() is thread-local, let's do defensive programming
  std::string PosixApi::GetErrorMessage(int errorNumber) {
    std::string buffer(256, '\0');
    for(;;) {

      // Try to obtain the error number. The return value of strerror_r() is different
      // between GNU and Posix. There's no reliable error return, so we reset errno for
      // the current thread and check it again after calling strerror_r()
      errno = 0;
      const char *posixErrorMessage = getStringFromStrErrorR(
        ::strerror_r(errorNumber, buffer.data(), buffer.length()), buffer
      );

      int errorNumberFromStrError = errno;
      if(errorNumberFromStrError == 0) {
        return std::string(posixErrorMessage);
      }

      // If the buffer was too small, try again with 1024 bytes, 4096 bytes and
      // 16384 bytes, then blow up.
      if(errorNumberFromStrError == ERANGE) {
        std::size_t bufferSize = buffer.size();
        if(bufferSize < 16384) {
          buffer.resize(bufferSize * 4);
          continue;
        }
      }

      // We failed to look up the error message. At least output the original
      // error number and remark that we weren't able to look up the error message.
      std::string errorMessage(u8"Error ");
      Nuclex::Support::Text::lexical_append(errorMessage, errorNumber);
      errorMessage.append(u8" (and error message lookup failed)");
      return errorMessage;

    } // for(;;)
  }
#endif

  // ------------------------------------------------------------------------------------------- //

  void PosixApi::ThrowExceptionForSystemError(const std::string &errorMessage, int errorNumber) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(PosixApi::GetErrorMessage(errorNumber));

    throw std::system_error(
      std::error_code(errorNumber, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // !defined(NUCLEX_AUDIO_WINDOWS)
