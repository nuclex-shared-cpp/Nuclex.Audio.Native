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

#include "./WavPackReader.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "./WavPackVirtualFileAdapter.h"
#include "../../Platform/WavPackApi.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackReader::SampleFormatFromModeAndBitsPerSample(
    int mode, int bitsPerSample
  ) {

    // Figure out the data format closest to the data stored by WavPack. Normally it
    // should be an exact match, but WavPack leaves room to store fewer bits, not only
    // for 24-bit formats. For the sake of robustness, we'll anticipate those, too.
    if((mode & MODE_FLOAT) != 0) {
      if(bitsPerSample >= 33) {
        return Nuclex::Audio::AudioSampleFormat::Float_64;
      } else {
        return Nuclex::Audio::AudioSampleFormat::Float_32;
      }
    } else {
      if(bitsPerSample >= 25) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_32;
      } else if(bitsPerSample >= 17) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_24;
      } else if(bitsPerSample >= 9) {
        return Nuclex::Audio::AudioSampleFormat::SignedInteger_16;
      } else {
        return Nuclex::Audio::AudioSampleFormat::UnsignedInteger_8;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  WavPackReader::WavPackReader(const std::shared_ptr<const VirtualFile> &file) :
    file(file),
    streamReader(),
    state(),
    context() {

    // Set up a WavPack stream reader with adapter methods that will perform all reads
    // on the provided virtual file.
    this->state = std::move(
      StreamAdapterFactory::CreateAdapterForReading(file, this->streamReader)
    );

    // Open the WavPack file, obtaining a WavPack context.The exception_ptr is checked
    // inside that WavPackApi wrapper, ensuring that the right exception surfaces if
    // either libwavpack reports an error or the virtual file throws.
    this->context = Platform::WavPackApi::OpenStreamReaderInput(
      this->state->Error, // exception_ptr that will receive VirtualFile exceptions
      this->streamReader,
      this->state.get() // passed to all IO callbacks as void pointer
    );

    // The OpenStreamReaderInput() method will already have checked for errors,
    // but if some file access error happened that libwavpack deemed non-fatal,
    // we still want to throw it - an exception in VirtualFile should always surface.
    StreamAdapterState::RethrowPotentialException(*state);

  }

  // ------------------------------------------------------------------------------------------- //

  WavPackReader::~WavPackReader() {}

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
