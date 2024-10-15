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

#include "OpusEncoderApi.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include <Nuclex/Support/Text/StringConverter.h>

#include <stdexcept>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns an error message matching the specified opusenc error code</summary>
  /// <param name="errorCode">Error code for which an error message will be returned</param>
  /// <returns>A string stating the error indicated by the error code</returns>
  std::string stringFromOpusEncErrorCode(int errorCode) {
    switch(errorCode) {
      case OPE_OK: { return u8"No error"; }
      case OPE_BAD_ARG: { return u8"An argument has an invalid value"; }
      case OPE_INTERNAL_ERROR: { return u8"An internal error occurred"; }
      case OPE_UNIMPLEMENTED: { return u8"Format version or chosen options not supported"; }
      case OPE_ALLOC_FAIL: { return u8"Out of memory"; }
      case OPE_CANNOT_OPEN: { return u8"Unable to open file"; }
      case OPE_TOO_LATE: { return u8"Option can not be changed anymore at this point"; }
      case OPE_INVALID_PICTURE: { return u8"Picture is invalid"; }
      case OPE_INVALID_ICON: { return u8"Icon is invalid"; }
      case OPE_WRITE_FAIL: { return u8"Error during file write attempt"; }
      case OPE_CLOSE_FAIL: { return u8"Unable to close file"; }
      default: { return u8"An unspecified error occurred"; }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::OggOpusComments> OpusEncoderApi::CreateComments() {
    ::OggOpusComments *opusComments = ::ope_comments_create();
    if(unlikely(opusComments == nullptr)) {
      throw std::runtime_error(u8"Could not create Ogg/Opus comment record");
    }

    return std::shared_ptr<::OggOpusComments>(opusComments, &::ope_comments_destroy);
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::OggOpusEnc> OpusEncoderApi::CreateFromCallbacks(
    void *state,
    const ::OpusEncCallbacks *callbacks,
    std::size_t sampleRate,
    std::size_t channelCount,
    const std::shared_ptr<::OggOpusComments> comments /* = CreateComments() */
  ) {
    int family = (channelCount < 3) ? 0 : 1; // 0 = stereo, 1 = surround
    int errorCode = 0;

    ::OggOpusEnc *opusEncoder = ::ope_encoder_create_callbacks(
      callbacks,
      state,
      comments.get(),
      static_cast<::opus_int32>(sampleRate),
      static_cast<int>(channelCount),
      family,
      &errorCode
    );
    if(unlikely(opusEncoder == nullptr)) {
      std::string message(u8"Error creating a new Opus encoder: ", 35);
      message.append(stringFromOpusEncErrorCode(errorCode));
      throw std::runtime_error(message);
    }

    return std::shared_ptr<::OggOpusEnc>(opusEncoder, &::ope_encoder_destroy);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusEncoderApi::ControlEncoder(
    const std::shared_ptr<::OggOpusEnc> &encoder, int request, int value
  ) {
    int result = ::ope_encoder_ctl(encoder.get(), request, value);
    if(unlikely(result != OPE_OK)) {
      std::string message(u8"Error sending Opus encoder control message: ", 44);
      message.append(stringFromOpusEncErrorCode(result));
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusEncoderApi::WriteFloats(
    const std::shared_ptr<::OggOpusEnc> &encoder, const float *samples, std::size_t frameCount
  ) {
    int result = ::ope_encoder_write_float(encoder.get(), samples, frameCount);
    if(unlikely(result != OPE_OK)) {
      std::string message(u8"Error feeding audio samples to the Opus encoder: ", 49);
      message.append(stringFromOpusEncErrorCode(result));
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusEncoderApi::Drain(
    const std::shared_ptr<::OggOpusEnc> &encoder
  ) {
    int result = ::ope_encoder_drain(encoder.get());
    if(result != OPE_OK) {
      std::string message(u8"Error draining the final samples from the encoder: ", 51);
      message.append(stringFromOpusEncErrorCode(result));
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
