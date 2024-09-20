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

#include "./ResourceDirectoryLocator.h"

#include <Nuclex/Support/Threading/Process.h> // for Process::GetExecutableDirectory()

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  std::string GetResourcesDirectory() {
#if defined(NUCLEX_AUDIO_WINDOWS)
    constexpr const char DirectorySeparator = '\\';
#else
    constexpr const char DirectorySeparator = '/';
#endif

    std::string executableDirectory = (
      Nuclex::Support::Threading::Process::GetExecutableDirectory()
    );

    std::string::size_type length = executableDirectory.length();
    std::string_view baseDirectory(executableDirectory.data(), length - 1);

    std::string_view::size_type cutOffIndex = baseDirectory.rfind(DirectorySeparator);
    assert(cutOffIndex != std::string::npos);

    baseDirectory = baseDirectory.substr(0, cutOffIndex);

    cutOffIndex = baseDirectory.rfind(DirectorySeparator);
    assert(cutOffIndex != std::string::npos);

    return (
      std::string(baseDirectory.substr(0, cutOffIndex + 1)) +
      "Resources" +
      DirectorySeparator
    );
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio