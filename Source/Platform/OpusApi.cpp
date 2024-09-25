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

  /// <summary>Returns an error message matching the specified opusfile error code</summary>
  /// <param name="errorCode">Error code for which an error message will be returned</param>
  /// <returns>A string stating the error indicated by the error code</returns>
  std::string stringFromOpusFileErrorCode(int errorCode) {
    switch(errorCode) {
      case OP_FALSE: { return u8"A request did not succeed"; }
      case OP_EOF: { return u8"Unexpected end of file"; }
      case OP_HOLE: { return u8"Page sequence number skipped, file corrupt"; }
      case OP_EREAD: { return u8"Read, seek or tell on a file has failed"; }
      case OP_EFAULT: { return u8"Internal library error, out of memory or null pointer"; }
      case OP_EIMPL: { return u8"Stream is using an unsupported feature"; }
      case OP_EINVAL: { return u8"Function called with invalid parameters"; }
      case OP_ENOTFORMAT: { return u8"Stream did not contain OGG/Opus data"; }
      case OP_EBADHEADER: { return u8"Required header missing or format violation"; }
      case OP_EVERSION: { return u8"Unsupported stream version"; }
      case OP_ENOTAUDIO: { return u8"OGG stream did not contain Opus audio data"; }
      case OP_EBADPACKET: { return u8"Packed failed to decode properly"; }
      case OP_EBADLINK: { return u8"Error navigating between linked audio streams"; }
      case OP_ENOSEEK: { return u8"Operation requires seeking but stream is unseekable"; }
      case OP_EBADTIMESTAMP: { return u8"Start or end timestamp of stream was invalid"; }
      default: { return u8"An unspecified error occurred"; }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::OggOpusFile> OpusApi::OpenFromCallbacks(
    const std::exception_ptr &rootCauseException,
    void *state,
    const ::OpusFileCallbacks *callbacks,
    const std::uint8_t *initialBytes /* = nullptr */,
    std::size_t initialByteCount /* = 0 */
  ) {
    int errorCode = 0;
    ::OggOpusFile *opusFile = ::op_open_callbacks(
      state, callbacks, initialBytes, initialByteCount, &errorCode
    );
    if(unlikely(opusFile == nullptr)) {

      // If something happened reading from the virtual file, that is the root cause
      // exception and will be reported above whatever error it caused in libopusfile.
      if(unlikely(static_cast<bool>(rootCauseException))) {
        std::rethrow_exception(rootCauseException);
      }

      std::string message(u8"Error opening virtual file via libopusfile: ", 44);
      message.append(stringFromOpusFileErrorCode(errorCode));
      throw std::runtime_error(message);

    }

    //::op_set_gain_offset(opusFile, OP_HEADER_GAIN, 0);

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

  std::uint64_t OpusApi::CountSamples(
    const std::shared_ptr<::OggOpusFile> &opusFile, int linkIndex /* = -1 */
  ) {
    ::ogg_int64_t sampleCountOrErrorCode = ::op_pcm_total(opusFile.get(), linkIndex);
    if(sampleCountOrErrorCode < 0) {
      std::string message(u8"Error getting total pcm sample count via libopusfile: ", 54);
      message.append(stringFromOpusFileErrorCode(static_cast<int>(sampleCountOrErrorCode)));
      throw std::runtime_error(message);
    }

    return static_cast<std::uint64_t>(sampleCountOrErrorCode);
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t OpusApi::GetRawContainerSize(
    const std::shared_ptr<::OggOpusFile> &opusFile, int linkIndex /* = -1 */
  ) {
    ::ogg_int64_t containerSizeOrErrorCode = ::op_raw_total(opusFile.get(), linkIndex);
    if(unlikely(containerSizeOrErrorCode < 0)) {
      std::string message(u8"Error getting OGG raw total size via libopusfile: ", 50);
      message.append(stringFromOpusFileErrorCode(static_cast<int>(containerSizeOrErrorCode)));
      throw std::runtime_error(message);
    }

    return static_cast<std::uint64_t>(containerSizeOrErrorCode);
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  std::uint64_t OpusApi::GetRawStreamSize(
    const std::shared_ptr<::OggOpusFile> &opusFile, int linkIndex /* = -1 */
  ) {
    std::uint64_t totalStreamSize = 0;
    // Step through all OGG pages and sum up the size of one stream only
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  void OpusApi::PcmSeek(
    const std::shared_ptr<::OggOpusFile> &opusFile,
    std::int64_t pcmOffset // This is a signed integer in in the opusfile API...
  ) {
    int result = ::op_pcm_seek(opusFile.get(), pcmOffset);
    if(unlikely(result != 0)) {
      std::string message(u8"Error seeking within the Opus audio file: ", 42);
      message.append(stringFromOpusFileErrorCode(result));
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t OpusApi::Read(
    const std::shared_ptr<::OggOpusFile> &opusFile,
    std::int16_t *buffer, int bufferSize,
    int linkIndex /* = -1 */
  ) {
    int result;
    if(linkIndex == -1) {
      result = ::op_read(opusFile.get(), buffer, bufferSize, nullptr);
    } else {
      result = ::op_read(opusFile.get(), buffer, bufferSize, &linkIndex);
    }
    if(unlikely(result < 0)) {
      std::string message(u8"Error decoding samples from Opus audio file: ", 45);
      message.append(stringFromOpusFileErrorCode(result));
      throw std::runtime_error(message);
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t OpusApi::ReadFloat(
    const std::shared_ptr<::OggOpusFile> &opusFile,
    float *buffer, int bufferSize,
    int linkIndex /* = -1 */
  ) {
    int result;
    if(linkIndex == -1) {
      result = ::op_read_float(opusFile.get(), buffer, bufferSize, nullptr);
    } else {
      result = ::op_read_float(opusFile.get(), buffer, bufferSize, &linkIndex);
    }
    if(unlikely(result < 0)) {
      std::string message(u8"Error decoding samples from Opus audio file: ", 45);
      message.append(stringFromOpusFileErrorCode(result));
      throw std::runtime_error(message);
    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
