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
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/VorbisApi.h" // for VorbisApi

#include "Nuclex/Audio/Errors/CorruptedFileError.h"

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

    // Set up the libvorbisfile callbacks with adapter methods that will perform all reads
    // on the user-provided virtual file.
    this->state = (
      FileAdapterFactory::CreateAdapterForReading(file, this->fileCallbacks)
    );

    // Open the Vorbis file, obtaining a OggVorbisFile instance. Everything inside
    // this scope is just error plumbing code, ensuring that the right exception
    // surfaces if either libvorbisfile reports an error or the virtual file throws.
    this->vorbisFile = Platform::VorbisApi::OpenFromCallbacks(
      state->Error,
      state.get(),
      fileCallbacks
    );

    // The OpenFromCallbacks() method will already have checked for errors,
    // but if some file access error happened that libvorbisfile deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    FileAdapterState::RethrowPotentialException(*state);

    // Vorbis audio streams can be chained together (sequentially and not in the sense of
    // interleaving it as another stream in the OGG container). This would mean that
    // the audio stream properties (i.e. channel count, sample rate) might change while
    // we are decoding...
    //
    std::size_t streamCount = Platform::VorbisApi::CountStreams(this->vorbisFile);
    if(streamCount != 1) {
      throw std::runtime_error(u8"Multi-stream Vorbis files are not supported");
    }

    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);
    this->channelCount = info.channels;

  }

  // ------------------------------------------------------------------------------------------- //

  VorbisReader::~VorbisReader() {}

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::ReadMetadata(TrackInfo &target) {
    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);

    // For Vorbis files, the mapping family is always 0 (says the Vorbis 1 specification)
    target.ChannelCount = info.channels;
    target.ChannelPlacements = (
      Shared::ChannelOrderFactory::ChannelPlacementFromVorbisFamilyAndCount(
        0, target.ChannelCount
      )
    );

    target.SampleRate = info.rate;

    ::ogg_int64_t totalSampleCount = Platform::VorbisApi::CountPcmSamples(this->vorbisFile);

    target.Duration = std::chrono::microseconds(
      totalSampleCount * 1'000'000 / target.SampleRate
    );

    target.BitsPerSample = 15; // come up with a silly, wrong approximation formula here

    target.SampleFormat = AudioSampleFormat::Float_32;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::CountTotalFrames() const {
    return Platform::VorbisApi::CountPcmSamples(this->vorbisFile);
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> VorbisReader::GetChannelOrder() const {
    const ::vorbis_info &info = Platform::VorbisApi::GetStreamInformation(this->vorbisFile);

    return Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(
      0, info.channels
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t VorbisReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::Seek(std::uint64_t frameIndex) {
    Platform::VorbisApi::Seek(
      this->state->Error,
      this->vorbisFile,
      frameIndex
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::DecodeSeparated(float **&buffer, std::size_t frameCount) {
    while(frameCount >= 1) {

      int streamIndex = -1;
      std::size_t decodedFrameCount = (
        Platform::VorbisApi::ReadFloat(
          this->state->Error,
          this->vorbisFile,
          buffer,
          frameCount, // * this->channelCount,
          streamIndex
        )
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Vorbis file. File truncated?"
        );
      }
      if(streamIndex != 0) {
        throw std::runtime_error(
          u8"Vorbis playback encountered a second stream. Multi-stream files are not supported."
        );
      }

      //float *left = samples[0];
      //float *right = samples[1];

      this->frameCursor += decodedFrameCount;

      buffer += decodedFrameCount * this->channelCount;
      if(decodedFrameCount > frameCount) {
        assert((frameCount >= decodedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }
      frameCount -= decodedFrameCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void VorbisReader::DecodeInterleaved(float *buffer, std::size_t frameCount) {
    while(frameCount >= 1) {

      float **samples = nullptr;
      int streamIndex = -1;
      std::size_t decodedFrameCount = (
        Platform::VorbisApi::ReadFloat(
          this->state->Error,
          this->vorbisFile,
          samples,
          frameCount, // * this->channelCount,
          streamIndex
        )
      );
      if(decodedFrameCount == 0) {
        throw Errors::CorruptedFileError(
          u8"Unexpected end of audio stream decoding Vorbis file. File truncated?"
        );
      }
      if(streamIndex != 0) {
        throw std::runtime_error(
          u8"Vorbis playback encountered a second stream. Multi-stream files are not supported."
        );
      }

      for(std::size_t frameIndex = 0; frameIndex < decodedFrameCount; ++frameIndex) {
        for(std::size_t channelIndex = 0; channelIndex < this->channelCount; ++channelIndex) {
          *buffer = samples[channelIndex][frameIndex];
          ++buffer;
        }
      }

      this->frameCursor += decodedFrameCount;
      frameCount -= decodedFrameCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Vorbis

#endif // defined(NUCLEX_AUDIO_HAVE_VORBIS)
