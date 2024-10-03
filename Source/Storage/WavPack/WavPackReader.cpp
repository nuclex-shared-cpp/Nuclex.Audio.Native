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

#include "Nuclex/Audio/TrackInfo.h"

#include "./WavPackVirtualFileAdapter.h"
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/WavPackApi.h"

// This is a bit "nasty" - for efficient decoding, we want to decode
// from block boundaries and by whole blocks. However, that information is
// not readily available from the public libwavpack API. One option would
// be to parse the WavPack file's structures ourselves, but that would
// possibly make it harder to update -- if the format changes, we would
// read garbage and have to dig down into the format again. By accessing
// some private WavPack fields, we get a compilation error if those change
// and the file doesn't have tp be parsed twice.
#define NUCLEX_AUDIO_ACCESS_WAVPACK_INTERNALS 1

#if defined(NUCLEX_AUDIO_ACCESS_WAVPACK_INTERNALS)
#include <../src/wavpack_local.h>
#endif

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
      return Nuclex::Audio::AudioSampleFormat::Float_32;
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
    context(),
    mode(0),
    bitsPerSample(0),
    bytesPerSample(0),
    channelCount(0),
    sampleRate(0),
    frameCursor(0),
    decodeInterleavedUint8(),
    decodeInterleavedInt16(),
    decodeInterleavedInt32(),
    decodeInterleavedFloat(),
    decodeInterleavedDouble(),
    decodeSeparatedUint8(),
    decodeSeparatedInt16(),
    decodeSeparatedInt32(),
    decodeSeparatedFloat(),
    decodeSeparatedDouble() {

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

    this->mode = Platform::WavPackApi::GetMode(this->context);
    this->bitsPerSample = Platform::WavPackApi::GetBitsPerSample(context);
    this->bytesPerSample = Platform::WavPackApi::GetBytesPerSample(context);
    this->channelCount = Platform::WavPackApi::GetNumChannels(context);
    this->sampleRate = Platform::WavPackApi::GetSampleRate(context);
  }

  // ------------------------------------------------------------------------------------------- //

  WavPackReader::~WavPackReader() {}

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::ReadMetadata(TrackInfo &target) {
    target.ChannelCount = this->channelCount;

    target.ChannelPlacements = static_cast<ChannelPlacement>(
      Platform::WavPackApi::GetChannelMask(this->context)
    );

    target.SampleFormat = GetSampleFormat();

    target.SampleRate = Platform::WavPackApi::GetSampleRate(this->context);
    target.BitsPerSample = this->bitsPerSample;

    std::uint64_t totalSampleCount = Platform::WavPackApi::GetNumSamples64(this->context);
    target.Duration = std::chrono::microseconds(
      totalSampleCount * 1'000'000 / target.SampleRate
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WavPackReader::CountTotalFrames() const {
    return Platform::WavPackApi::GetNumSamples64(this->context);
  }

  // ------------------------------------------------------------------------------------------- //

  AudioSampleFormat WavPackReader::GetSampleFormat() const {
    return SampleFormatFromModeAndBitsPerSample(this->mode, this->bitsPerSample);
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> WavPackReader::GetChannelOrder() const {

    int wavPackChannelCount = Platform::WavPackApi::GetNumChannels(this->context);
    int wavPackChannelMask = Platform::WavPackApi::GetChannelMask(this->context);

    // Just like Waveform, in WavPack the channel order matches the order of the flag bits.
    return Shared::ChannelOrderFactory::FromWaveformatExtensibleLayout(
      static_cast<std::size_t>(wavPackChannelCount),
      static_cast<ChannelPlacement>(wavPackChannelMask)
    );

  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WavPackReader::GetCurrentBlockSize() const {
    #if defined(NUCLEX_AUDIO_ACCESS_WAVPACK_INTERNALS)
    ::WavpackStream *wavpackStream = this->context->streams[0];
    if(wavpackStream != nullptr) {
      std::size_t sampleCountInBlock = static_cast<std::size_t>(
        wavpackStream->wphdr.block_samples
      );
      if((sampleCountInBlock >= 512) && (sampleCountInBlock < 24001)) {
        return sampleCountInBlock;
      }
    }
    #endif

    // This should be a "reasonable" default for when we don't want to peek into
    // the internal structures of libwavpack. In my test files, block size seems
    // to always be half a second, but I have not investigated in-depth if that
    // always holds up or if that is just a side effect of my test audio files
    // having neat, easy-to-compress sine waves.
    return this->sampleRate / 2;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t WavPackReader::GetFrameCursorPosition() const {
    return this->frameCursor;
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::PrepareForDecoding() {
    if(mode & MODE_FLOAT) {
      this->decodeInterleavedUint8 = (
        &WavPackReader::decodeInterleavedAndConvert<std::uint8_t, false, 0>
      );
    } else if(this->bitsPerSample < 8) { // We just assume it's not under 4...
      this->decodeInterleavedUint8 = (
        &WavPackReader::decodeInterleavedAndConvert<std::uint8_t, false, 2>
      );
    } else if(this->bitsPerSample < 9) { // Exact match, specify 1 for no repeats
      this->decodeInterleavedUint8 = (
        &WavPackReader::decodeInterleavedAndConvert<std::uint8_t, false, 1>
      );
    } else { // Decodes to 9 or more bits, specify -1 to indicate truncation
      this->decodeInterleavedUint8 = (
        &WavPackReader::decodeInterleavedAndConvert<std::uint8_t, false, -1>
      );
    }

    if(mode & MODE_FLOAT) {
      this->decodeInterleavedInt16 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int16_t, false, 0>
      );
    } else if(this->bitsPerSample < 9) { // 8 bits or fewer need two repeats
      this->decodeInterleavedInt16 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int16_t, false, 3>
      );
    } else if(this->bitsPerSample < 16) { // 15 bits or fewer need one repeat
      this->decodeInterleavedInt16 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int16_t, false, 2>
      );
    } else if(this->bitsPerSample < 17) { // Exact match, specify 1 for no repeats
      this->decodeInterleavedInt16 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int16_t, false, 1>
      );
    } else { // Decodes to 17 or more bits, specify -1 to indicate truncation
      this->decodeInterleavedInt16 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int16_t, true, -1>
      );
    }

    if(mode & MODE_FLOAT) {
      this->decodeInterleavedInt32 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int32_t, false, 0>
      );
    } else if(this->bitsPerSample < 17) { // 16 bits or fewer needs two repeats
      this->decodeInterleavedInt32 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int32_t, false, 3>
      );
    } else if(this->bitsPerSample < 32) { // fewer than 32 bits needs one repeat
      this->decodeInterleavedInt32 = (
        &WavPackReader::decodeInterleavedAndConvert<std::int32_t, true, 2>
      );
    }

    if(mode & MODE_FLOAT) {
      this->decodeInterleavedDouble = (
        &WavPackReader::decodeInterleavedAndConvert<double, false, 0>
      );
    } else if(this->bitsPerSample < 17) {
      this->decodeInterleavedFloat = (
        &WavPackReader::decodeInterleavedAndConvert<float, false>
      );
      this->decodeInterleavedDouble = (
        &WavPackReader::decodeInterleavedAndConvert<double, false>
      );
    } else {
      this->decodeInterleavedFloat = (
        &WavPackReader::decodeInterleavedAndConvert<float, true>
      );
      this->decodeInterleavedDouble = (
        &WavPackReader::decodeInterleavedAndConvert<double, true>
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WavPackReader::Seek(std::uint64_t frameIndex) {

    // ISSUE: SeekSample64() is documented as bringing the context into an invalid state
    // when it returns an error and that no further operations besides closing
    // the WavPack file are supported after that. Should we intervene to guarantee
    // correct behavior or should we leave it up to random chance / the user?
    Platform::WavPackApi::SeekSample(this->state->Error, this->context, frameIndex);
    this->frameCursor = frameIndex;
    //StreamAdapterState::RethrowPotentialException(*state);

  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<std::uint8_t>(
    std::uint8_t *target, std::size_t frameCount
  ) {
    (this->*decodeInterleavedUint8)(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<std::int16_t>(
    std::int16_t *target, std::size_t frameCount
  ) {
    (this->*decodeInterleavedInt16)(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<std::int32_t>(
    std::int32_t *target, std::size_t frameCount
  ) {
    if((mode & MODE_FLOAT) || (this->bitsPerSample != 32)) {
      (this->*decodeInterleavedInt32)(target, frameCount);
    } else { // 32 bits (only case remaining) can be decoded directly into user buffer
      std::uint32_t unpackedFrameCount = Platform::WavPackApi::UnpackSamples(
        this->state->Error, // exception_ptr that will receive VirtualFile exceptions
        this->context,
        target,
        static_cast<std::uint32_t>(frameCount)
      );

      this->frameCursor += unpackedFrameCount;

      if(unpackedFrameCount != frameCount) {
        throw std::runtime_error(
          u8"libwavpack unpacked a different number of samples than was requested. "
          u8"Truncated file?"
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<float>(
    float *target, std::size_t frameCount
  ) {
    if(mode & MODE_FLOAT) {

      // If the audio file contains floating point data and the caller wants floats,
      // just let libwavpack unpack it all directly into the caller-provided buffer.
      //
      // The parameter type is std::int32_t however -- so unless the caller allocates
      // a buffer of char-based types, are we promoting a C++ aliasing violation here?
      std::uint32_t unpackedFrameCount = Platform::WavPackApi::UnpackSamples(
        this->state->Error, // exception_ptr that will receive VirtualFile exceptions
        this->context,
        reinterpret_cast<std::int32_t *>(target), // C++ strict aliasing violation or not?
        static_cast<std::uint32_t>(frameCount)
      );

      this->frameCursor += unpackedFrameCount;

      if(unpackedFrameCount != frameCount) {
        throw std::runtime_error(
          u8"libwavpack unpacked a different number of samples than was requested. "
          u8"Truncated file?"
        );
      }

    } else {
      (this->*decodeInterleavedFloat)(target, frameCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<double>(
    double *target, std::size_t frameCount
  ) {
    (this->*decodeInterleavedDouble)(target, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeSeparated<std::uint8_t>(
    std::uint8_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeSeparated<std::int16_t>(
    std::int16_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeSeparated<std::int32_t>(
    std::int32_t *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeSeparated<float>(
    float *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeSeparated<double>(
    double *targets[], std::size_t frameCount
  ) {
    decodeInterleavedConvertAndSeparate(targets, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#include "WavPackReader.Decoding.inl"

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
