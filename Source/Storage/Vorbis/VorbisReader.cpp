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

#include "./VorbisReader.h"

#if defined(NUCLEX_AUDIO_HAVE_VORBIS)

#include "./VorbisVirtualFileAdapter.h" // for FileAdapterFactory
#include "../../Platform/VorbisApi.h" // for VorbisApi

#include "Nuclex/Audio/TrackInfo.h"

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Vorbis {

  // ------------------------------------------------------------------------------------------- //

  VorbisReader::VorbisReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    fileCallbacks(),
    state(),
    vorbisFile(),
    channelCount(0),
    frameCursor(0) {

    // Set up the libopusfile callbacks with adapter methods that will perform all reads
    // on the user-provided virtual file.
    this->state = (
      FileAdapterFactory::CreateAdapterForReading(file, this->fileCallbacks)
    );

    // Open the Vorbis file, obtaining a OggVorbisFile instance. Everything inside
    // this scope is just error plumbing code, ensuring that the right exception
    // surfaces if either libopusfile reports an error or the virtual file throws.
    this->vorbisFile = Platform::VorbisApi::OpenFromCallbacks(
      state->Error,
      state.get(),
      fileCallbacks
    );

    // The OpenFromCallbacks() method will already have checked for errors,
    // but if some file access error happened that libopusfile deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    FileAdapterState::RethrowPotentialException(*state);

    // Vorbis audio streams can be chained together (sequentially and not in the sense of
    // interleaving it as another stream in the OGG container). This would mean that
    // the audio stream properties (i.e. channel count, sample rate) might change while
    // we are decoding...
    //
    std::size_t linkCount = Platform::VorbisApi::CountStreams(this->vorbisFile);
    if(linkCount != 1) {
      throw std::runtime_error(u8"Multi-stream Vorbis files are not supported");
    }

  }

  // ------------------------------------------------------------------------------------------- //

  VorbisReader::~VorbisReader() {}

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::ReadMetadata(TrackInfo &target) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::CountTotalFrames() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> VorbisReader::GetChannelOrder() const {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::Seek(std::uint64_t frameIndex) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::DecodeInterleaved(float *buffer, std::size_t frameCount) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
