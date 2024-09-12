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

#include "FlacApi.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include <Nuclex/Support/Text/StringConverter.h>

#include <stdexcept>

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::FLAC__StreamDecoder> FlacApi::NewStreamDecoder() {
    ::FLAC__StreamDecoder *decoder = ::FLAC__stream_decoder_new();
    if(decoder == nullptr) {
      throw std::runtime_error(u8"Unable to allocate new FLAC stream decoder");
    }

    return std::shared_ptr<::FLAC__StreamDecoder>(
      decoder, &::FLAC__stream_decoder_delete
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacApi::EnableMd5Checking(
    const std::shared_ptr<::FLAC__StreamDecoder> &decoder, bool enable /* = true */
  ) {
    ::FLAC__stream_decoder_set_md5_checking(decoder.get(), enable ? 1 : 0);
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacApi::InitFile(
    const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
    const char *path,
    ::FLAC__StreamDecoderWriteCallback writeCallback,
    ::FLAC__StreamDecoderMetadataCallback metadataCallback,
    ::FLAC__StreamDecoderErrorCallback errorCallback,
    void *clientData
  ) {
    ::FLAC__StreamDecoderInitStatus result = ::FLAC__stream_decoder_init_file(
      decoder.get(),
      path,
      writeCallback,
      metadataCallback,
      errorCallback,
      clientData
    );
    if(unlikely(result != FLAC__STREAM_DECODER_INIT_STATUS_OK)) {
      std::string message(u8"Error trying to open the file via libflac: ", 43);
      message.append(::FLAC__StreamDecoderInitStatusString[result]);
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FlacApi::InitStream(
    const std::exception_ptr &rootCauseException,
    const std::shared_ptr<::FLAC__StreamDecoder> &decoder,
    ::FLAC__StreamDecoderReadCallback readCallback,
    ::FLAC__StreamDecoderSeekCallback seekCallback,
    ::FLAC__StreamDecoderTellCallback tellCallback,
    ::FLAC__StreamDecoderLengthCallback lengthCallback,
    ::FLAC__StreamDecoderEofCallback eofCallback,
    ::FLAC__StreamDecoderWriteCallback writeCallback,
    ::FLAC__StreamDecoderMetadataCallback metadataCallback,
    ::FLAC__StreamDecoderErrorCallback errorCallback,
    void *clientData
  ) {
    ::FLAC__StreamDecoderInitStatus result = ::FLAC__stream_decoder_init_stream(
      decoder.get(),
      readCallback,
      seekCallback,
      tellCallback,
      lengthCallback,
      eofCallback,
      writeCallback,
      metadataCallback,
      errorCallback,
      clientData
    );
    if(unlikely(result != FLAC__STREAM_DECODER_INIT_STATUS_OK)) {

      // If something happened reading from the virtual file, that is the root cause
      // exception and will be reported above whatever consequences it had inside libwavpack.
      if(unlikely(static_cast<bool>(rootCauseException))) {
        std::rethrow_exception(rootCauseException);
      }

      std::string message(u8"Error trying to open the virtual file via libflac: ", 51);
      message.append(::FLAC__StreamDecoderInitStatusString[result]);
      throw std::runtime_error(message);

    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)
