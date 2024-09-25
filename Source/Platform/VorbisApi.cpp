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

#include "VorbisApi.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include <Nuclex/Support/Text/StringConverter.h>

#include <stdexcept>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns an error message matching the specified vorbisfile error code</summary>
  /// <param name="errorCode">Error code for which an error message will be returned</param>
  /// <returns>A string stating the error indicated by the error code</returns>
  std::string stringFromVorbisFileErrorCode(int errorCode) {
    switch(errorCode) {
      case OV_FALSE: { return u8"A request did not succeed"; }
      case OV_EOF: { return u8"Unexpected end of file"; }
      case OV_HOLE: { return u8"Page sequence number skipped, file corrupt"; }
      case OV_EREAD: { return u8"Read, seek or tell on a file has failed"; }
      case OV_EFAULT: { return u8"Internal library error, out of memory or null pointer"; }
      case OV_EIMPL: { return u8"Stream is using an unsupported feature"; }
      case OV_EINVAL: { return u8"Function called with invalid parameters"; }
      case OV_ENOTVORBIS: { return u8"Stream did not contain OGG/Vorbis data"; }
      case OV_EBADHEADER: { return u8"Required header missing or format violation"; }
      case OV_EVERSION: { return u8"Unsupported stream version"; }
      case OV_ENOTAUDIO: { return u8"OGG stream did not contain Vorbis audio data"; }
      case OV_EBADPACKET: { return u8"Packed failed to decode properly"; }
      case OV_EBADLINK: { return u8"Error navigating between linked audio streams"; }
      case OV_ENOSEEK: { return u8"Operation requires seeking but stream is unseekable"; }
      default: { return u8"An unspecified error occurred"; }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::OggVorbis_File> VorbisApi::OpenFromCallbacks(
    const std::exception_ptr &rootCauseException,
    void *datasource,
    const ::ov_callbacks &callbacks,
    const char *initial, /* = nullptr, */
    long initialByteCount /* = 0 */
  ) {
    std::unique_ptr<::OggVorbis_File> vorbisFile = std::make_unique<::OggVorbis_File>();

    int errorCode = ::ov_open_callbacks(
      datasource, vorbisFile.get(), initial, initialByteCount, callbacks
    );
    if(unlikely(errorCode != 0)) {

      // If something happened reading from the virtual file, that is the root cause
      // exception and will be reported above whatever error it caused in libvorbisfile.
      if(unlikely(static_cast<bool>(rootCauseException))) {
        std::rethrow_exception(rootCauseException);
      }

      std::string message(u8"Error opening virtual file via libvorbisfile: ", 46);
      message.append(stringFromVorbisFileErrorCode(errorCode));
      throw std::runtime_error(message);

    }

    return std::shared_ptr<::OggVorbis_File>(vorbisFile.release(), &::ov_clear);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t VorbisApi::CountStreams(const std::shared_ptr<::OggVorbis_File> &vorbisFile) {
    // Can't fail, directly returns a structure member in the OggVorbis_File struct.
    //
    // Last checked in libvorbisfile, vorbisfile.c in 2024
    return static_cast<std::size_t>(::ov_streams(vorbisFile.get()));
  }

  // ------------------------------------------------------------------------------------------- //

  const ::vorbis_info &VorbisApi::GetStreamInformation(
    const std::shared_ptr<::OggVorbis_File> &vorbisFile,
    int streamIndex /* = -1 */
  ) {
    ::vorbis_info *info = ::ov_info(vorbisFile.get(), streamIndex);
    if(unlikely(info == nullptr)) {
      throw std::runtime_error(u8"Could not obtain Vorbis information read from audio file");
    }

    return *info;
  }

  // ------------------------------------------------------------------------------------------- //

  const ::vorbis_comment &VorbisApi::GetComments(
    const std::shared_ptr<::OggVorbis_File> &vorbisFile,
    int streamIndex /* = -1 */
  ) {
    ::vorbis_comment *comment = ::ov_comment(vorbisFile.get(), streamIndex);
    if(unlikely(comment == nullptr)) {
      throw std::runtime_error(u8"Could not obtain Vorbis information read from audio file");
    }

    return *comment;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisApi::CountPcmSamples(
    const std::shared_ptr<::OggVorbis_File> &vorbisFile,
    int streamIndex /* = -1 */
  ) {
    return ::ov_pcm_total(vorbisFile.get(), streamIndex);
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisApi::Seek(
    const std::exception_ptr &rootCauseException,
    const std::shared_ptr<::OggVorbis_File> &vorbisFile,
    ::ogg_int64_t sampleIndex
  ) {
    int result = ::ov_pcm_seek(vorbisFile.get(), sampleIndex);
    if(unlikely(result != 0)) {

      // If something happened reading from the virtual file, that is the root cause
      // exception and will be reported above whatever error it caused in libvorbisfile.
      if(unlikely(static_cast<bool>(rootCauseException))) {
        std::rethrow_exception(rootCauseException);
      }

      std::string message(u8"Error seeking in virtual file via libvorbisfile: ", 49);
      message.append(stringFromVorbisFileErrorCode(result));
      throw std::runtime_error(message);

    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t VorbisApi::ReadFloat(
    const std::exception_ptr &rootCauseException,
    const std::shared_ptr<::OggVorbis_File> &vorbisFile,
    float **&channelBuffers,
    int sampleCount,
    int &outStreamIndex
  ) {
    long result = ::ov_read_float(
      vorbisFile.get(), &channelBuffers, sampleCount, &outStreamIndex
    );
    if(unlikely(result < 0)) {

      // If something happened reading from the virtual file, that is the root cause
      // exception and will be reported above whatever error it caused in libvorbisfile.
      if(unlikely(static_cast<bool>(rootCauseException))) {
        std::rethrow_exception(rootCauseException);
      }

      std::string message(
        u8"Unable to decode audio samples from virtual file via libvorbisfile: ", 80
      );
      message.append(stringFromVorbisFileErrorCode(result));
      throw std::runtime_error(message);

    }

    return static_cast<std::size_t>(result);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Platform

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
