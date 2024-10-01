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
#include "Nuclex/Audio/Processing/Quantization.h"
#include "Nuclex/Audio/Processing/Reconstruction.h"

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
    frameCursor(0) {

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
    target.ChannelCount = Platform::WavPackApi::GetNumChannels(this->context);

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
    if(mode & MODE_FLOAT) {
      decodeInterleavedAndConvert<std::uint8_t, false, 0>(target, frameCount);
    } else { // If target is uint8_t, we simply assume that bits per sample will be longer.
      decodeInterleavedAndConvert<std::uint8_t, false, 1>(target, frameCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<std::int16_t>(
    std::int16_t *target, std::size_t frameCount
  ) {
    if(mode & MODE_FLOAT) {
      decodeInterleavedAndConvert<std::int16_t, false, 0>(target, frameCount);
    } else if(this->bitsPerSample < 16) {
      decodeInterleavedAndConvert<std::int16_t, false, 2>(target, frameCount);
    } else if(this->bitsPerSample < 17) {
      decodeInterleavedAndConvert<std::int16_t, false, 1>(target, frameCount);
    } else {
      decodeInterleavedAndConvert<std::int16_t, true, 1>(target, frameCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<std::int32_t>(
    std::int32_t *target, std::size_t frameCount
  ) {
    if(mode & MODE_FLOAT) {
      decodeInterleavedAndConvert<std::int32_t, false, 0>(target, frameCount);
    } else if(this->bitsPerSample < 17) {
      decodeInterleavedAndConvert<std::int32_t, false, 3>(target, frameCount);
    } else if(this->bitsPerSample < 32) {
      decodeInterleavedAndConvert<std::int32_t, true, 2>(target, frameCount);
    } else {
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

    } else if(this->bitsPerSample < 17) {
      decodeInterleavedAndConvert<float, false>(target, frameCount);
    } else {
      decodeInterleavedAndConvert<float, true>(target, frameCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> void WavPackReader::DecodeInterleaved<double>(
    double *target, std::size_t frameCount
  ) {
    if(mode & MODE_FLOAT) {
      decodeInterleavedAndConvert<double, false, 0>(target, frameCount);
    } else if(this->bitsPerSample < 17) {
      decodeInterleavedAndConvert<double, false>(target, frameCount);
    } else {
      decodeInterleavedAndConvert<double, true>(target, frameCount);
    }
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

  template<
    typename TSample,
    bool BitsPerSampleOver16 /* = false */,
    std::size_t WidenFactor /* = 1 */
  >
  void WavPackReader::decodeInterleavedAndConvert(TSample *target, std::size_t frameCount) {
    constexpr bool decodedSamplesAreFloat = (WidenFactor == 0);
    constexpr bool targetTypeIsFloat = (
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value
    );

    // Determine the number of frames to process per batch. If we're allowed to access
    // the internal data structure of libwavpack, we'll check on the current block being
    // decoded to see how much data we can decode in one go.
    std::size_t decodeChunkSize;
    {
#if defined(NUCLEX_AUDIO_ACCESS_WAVPACK_INTERNALS)
      decodeChunkSize = static_cast<std::size_t>(
        GET_BLOCK_INDEX(this->context->streams[0]->wphdr) +
        this->context->streams[0]->wphdr.block_samples -
        this->context->streams[0]->sample_index
      );
#else
      decodeChunkSize = this->sampleRate / 2;
#endif

      // Don't decode unlimited 
      while(12000 < decodeChunkSize) {
        decodeChunkSize >>= 1;
      }
    }

    // Allocate an intermedia buffer. In this variant, we use std::byte because
    // we're going to be decoding into it from libwavpack (either float or std::int32_t),
    std::vector<std::byte> decodeBuffer(
      decodeChunkSize * this->channelCount * sizeof(std::int32_t)
    );

    while(0 < frameCount) {

      // If this is the last chunk or if the user requested less frames than the chunk
      // size in the first place, limit the amount of data we request from libwavpack to
      // the number of frames we actually need.
      if(frameCount < decodeChunkSize) {
        decodeChunkSize = frameCount;
      }

      // Decode into our intermediate buffer. This will either be floats or std::int32_ts,
      // both written into an int32_t buffer. In the case of integers, the number of bits
      // actually carrying data varies, but for floats, it's always 32 bits.
      std::uint32_t unpackedFrameCount = Platform::WavPackApi::UnpackSamples(
        this->state->Error, // exception_ptr that will receive VirtualFile exceptions
        this->context,
        reinterpret_cast<std::int32_t *>(decodeBuffer.data()),
        static_cast<std::uint32_t>(decodeChunkSize)
      );

      // Record the new frame cursor position that the libwavpack decoding context will
      // now have assumes. This is important so we know when a seek is needed, even if
      // we should decide "fail" the decode on our side.
      frameCursor += unpackedFrameCount;

      if(unpackedFrameCount != decodeChunkSize) {
        throw std::runtime_error(
          u8"libwavpack unpacked a different number of samples than was requested. "
          u8"Truncated file?"
        );
      }

      // If the WavPack audio file contains floats and this method was called, the caller
      // wants them as something other than float, so we need to convert.
      std::size_t sampleCount = unpackedFrameCount * this->channelCount;

      if constexpr(decodedSamplesAreFloat) {
        float *decodedFloats = reinterpret_cast<float *>(decodeBuffer.data());

        // The target can only be a double or an integer type. If the target was float,
        // this method would never have been invoked and the decode would have happened
        // directly into the caller-provided buffer.
        if constexpr(std::is_same<TSample, double>::value) {

          // float to double -> just cast the floats
          for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
            target[sampleIndex] = static_cast<double>(decodedFloats[sampleIndex]);
          }
          target += sampleCount;

        } else { // if target type is ^^ double ^^ / vv integer vv

          // Determine the 'limit' type to use. This is about the target data type,
          // so we don't use 'BitsPerSampleOver16' here.
          typedef typename std::conditional<
            sizeof(TSample) < 17, float, double
          >::type LimitType;
          constexpr LimitType limit = static_cast<LimitType>(
            (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
          );
          while(3 < sampleCount) {
            std::int32_t scaled[4];
            Nuclex::Audio::Processing::Quantization::MultiplyToNearestInt32x4(
              decodedFloats, limit, scaled
            );
            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              target[0] = static_cast<TSample>(scaled[0] + 128);
              target[1] = static_cast<TSample>(scaled[1] + 128);
              target[2] = static_cast<TSample>(scaled[2] + 128);
              target[3] = static_cast<TSample>(scaled[3] + 128);
            } else {
              target[0] = static_cast<TSample>(scaled[0]);
              target[1] = static_cast<TSample>(scaled[1]);
              target[2] = static_cast<TSample>(scaled[2]);
              target[3] = static_cast<TSample>(scaled[3]);
            }
            decodedFloats += 4;
            target += 4;
            sampleCount -= 4;
          }
          while(0 < sampleCount) {
            if constexpr(std::is_same<TSample, std::uint8_t>::value) {
              target[0] = static_cast<TSample>(
                Nuclex::Audio::Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(decodedFloats[0]) * limit
                ) + 128
              );
            } else {
              target[0] = static_cast<TSample>(
                Nuclex::Audio::Processing::Quantization::NearestInt32(
                  static_cast<LimitType>(decodedFloats[0]) * limit
                )
              );
            }
            ++decodedFloats;
            ++target;
            --sampleCount;
          }
        } // if target type is double / integer

      } else { // if decoded data from libwavpack is ^^ float ^^ / vv int32 vv
        std::int32_t *decodedInts = reinterpret_cast<std::int32_t *>(decodeBuffer.data());

        // How libwavpack aligns samples is a bit unique. Going by the docs:
        //
        // - samples always fill complete bytes, leaving the high bytes empty
        //   (so for 16-bit audio, the two high bytes will be zero and for 24-bit audio,
        //   the highest byte will be zero)
        //
        // - if bits per sample is not a multiple of 8, the least significant bits will
        //   remain unused. I don't know if libwavpack just leaves them zeroed (so naive
        //   usage will loose a bit of amplitude range)
        //
        // This shift moves the sample bits to the right so the entire sample value is
        // in the least significant bits and the limit calculated above fits.
        //
        std::size_t shift = this->bytesPerSample * 8 - this->bitsPerSample;

        if constexpr(targetTypeIsFloat) {

          // For our signal reconstruction helpers, the limit can be double even if floats
          // are the output (all internal calculations will be performed using doubles then).
          typedef typename std::conditional<
            std::is_same<TSample, double>::value || BitsPerSampleOver16, double, float
          >::type LimitType;
          LimitType limit = static_cast<LimitType>(
            (std::uint32_t(1) << (this->bitsPerSample - 1)) - 1
          );

          while(3 < sampleCount) {
            std::int32_t shifted[4];
            shifted[0] = decodedInts[0] >> shift;
            shifted[1] = decodedInts[1] >> shift;
            shifted[2] = decodedInts[2] >> shift;
            shifted[3] = decodedInts[3] >> shift;
            Nuclex::Audio::Processing::Reconstruction::DivideInt32ToFloatx4(
              shifted, limit, target
            );
            decodedInts += 4;
            target += 4;
            sampleCount -= 4;
          }
          while(0 < sampleCount) {
            target[0] = static_cast<TSample>(
              Nuclex::Audio::Processing::Reconstruction::DivideInt32ToFloat(
                decodedInts[0] >> shift, limit
              )
            );
            ++decodedInts;
            ++target;
            --sampleCount;
          }

          target += sampleCount;

        } else { // if target type is ^^ double ^^ / vv integer vv

          if constexpr(std::is_same<TSample, std::int16_t>::value && BitsPerSampleOver16) {
            std::runtime_error(u8"Truncating samples to lower bit counts not implemented yet");
          } else { // if ^^ integer needs truncation ^^ / ^^ integer needs extension ^^

            // This should also be the branch entered when bits per sample matches exactly
            // (can only happen for 16-bit audio, where libwavpack will decode into 32-bit
            // integers and we want to return 16-bit integers).
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              target[sampleIndex] = static_cast<TSample>(decodedInts[sampleIndex]);
            }
            target += sampleCount;

            std::runtime_error(u8"Truncating samples to lower bit counts not implemented yet");
          }

        } // if target type is double / integer
      } // if decoded data is float / in t32

      // Buffer was filled by the sample conversion method above, update the buffer pointer
      // to point to where the next batch of samples needs to be written
      if(unpackedFrameCount > frameCount) {
        assert((frameCount >= unpackedFrameCount) && u8"Read stays within buffer bounds");
        break;
      }

      frameCount -= unpackedFrameCount;

    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void WavPackReader::decodeInterleavedConvertAndSeparate(
    TSample *targets[], std::size_t frameCount
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)
