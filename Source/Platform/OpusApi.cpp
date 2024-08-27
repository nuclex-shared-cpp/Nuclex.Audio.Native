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

#include "OpusApi.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <Nuclex/Support/Text/StringConverter.h>

#include <stdexcept>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::OggOpusFile> OpusApi::OpenFromCallbacks(
    const Nuclex::Support::Events::Delegate<void()> &throwRootCauseException,
    void *state,
    const ::OpusFileCallbacks *callbacks,
    const std::uint8_t *initialBytes /* = nullptr */,
    std::size_t initialByteCount /* = 0 */
  ) {
    int errorCode = 0;
    ::OggOpusFile *opusFile = ::op_open_callbacks(
      state, callbacks, initialBytes, initialByteCount, &errorCode
    );
    if(opusFile == nullptr) {
      throwRootCauseException();

      std::string message(u8"Error opening virtual file via libopusfile: ", 44);
      message.append(::opus_strerror(errorCode));
      throw std::runtime_error(message);
    }

    return std::shared_ptr<::OggOpusFile>(opusFile, &::op_free);
  }

  // ------------------------------------------------------------------------------------------- //

  const OpusHead &OpusApi::GetHeader(
    const std::shared_ptr<::OggOpusFile> &opusFile, int linkIndex /* = -1 */
  ) {
    // The op_head() method simply looks up an array and clamps the link index to the number
    // of links in the OGG container (and -1 means the current link). So it cannot ever fail,
    // though if the OGG container somehow had zero links or the current link was invalid,
    // it would invoke undefined behavior, most likely returning an out-of-bounds pointer.
    //
    // Last checked in libopusfile, opusfile.c in 2024.
    return *::op_head(opusFile.get(), linkIndex);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t OpusApi::CountLinks(const std::shared_ptr<::OggOpusFile> &opusFile) {
    // Can't fail either, directly returns a structure member in the OggOpusfile struct.
    //
    // Last checked in libopusfile, opusfile.c in 2024
    return static_cast<std::size_t>(::op_link_count(opusFile.get()));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
