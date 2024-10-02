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

#if !defined(NUCLEX_AUDIO_SOURCE)
  #error This file must be included from WavPackReader.cpp directly
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Comes up with a good number of frames to process per batch</summary>
  /// <param name="requestedFrameCount">Total number of frames that need to be decoded</param>
  std::size_t estimateDecodeChunkSize(
    const std::shared_ptr<::WavpackContext> &context, std::size_t requestedFrameCount
  ) {

    // If we're allowed to access the internal data structure of libwavpack,
    // we'll check the current block being decoded to see how much data we
    // can decode in one go.
    std::size_t decodeChunkSize;
    {
#if defined(NUCLEX_AUDIO_ACCESS_WAVPACK_INTERNALS)
      decodeChunkSize = static_cast<std::size_t>(
        GET_BLOCK_INDEX(context->streams[0]->wphdr) +
        context->streams[0]->wphdr.block_samples -
        context->streams[0]->sample_index
      );
#else
      decodeChunkSize = this->sampleRate / 2;
#endif

      // Don't decode an unlimited number of samples, we want t
      while(12000 < decodeChunkSize) {
        decodeChunkSize >>= 1;
      }
    }

    // Also estimate a chunk size from the number of frames we should decode.
    while(12000 < requestedFrameCount) {
      requestedFrameCount >>= 2;
    }

    // Use the larger of the two values and round it up to a multiple of 4,
    // which will help avoid single-value processing of SSE2 SIMD code is used.
    if(decodeChunkSize < requestedFrameCount) {
      return (requestedFrameCount + 3) / 4 * 4;
    } else {
      return (decodeChunkSize + 3) / 4 * 4;
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  template<
    typename TSample,
    bool BitsPerSampleOver16 /* = false */,
    int WidenFactor /* = 1 */
  >
  void WavPackReader::decodeInterleavedAndConvert(TSample *target, std::size_t frameCount) {
    constexpr bool decodedSamplesAreFloat = (WidenFactor == 0); // per convention
    constexpr bool targetIntegerHasFewerBits = (WidenFactor == -1);
    constexpr bool decodedIntegerMatchesTargetBits = (WidenFactor == 1);
    constexpr bool targetTypeIsFloat = (
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value
    );

    // Allocate an intermedia buffer. In this variant, we use std::byte because
    // we're going to be decoding into it from libwavpack (either float or std::int32_t),
    std::size_t decodeChunkSize = estimateDecodeChunkSize(this->context, frameCount);
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

      // Decode into our intermediate buffer. All samples will be 32-bit values, but
      // This will either be std::int32_t (with all or some bits occupied) or floats.
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
#pragma region Convert floats to doubles or integers
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

          // Determine the 'limit' type to use for the numeric range in the output type.
          // This is about the target data type, so we don't use 'BitsPerSampleOver16' here.
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
#pragma endregion // Convert floats to doubles or integers
      } else { // if decoded data from libwavpack is ^^ float ^^ / vv int32 vv
#pragma region Convert integers to floats, doubles or integers
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

          // For our signal reconstruction helpers, the limit can be a 'double' even if floats
          // are the output (all internal calculations will be performed using 'double' then).
          typedef typename std::conditional<
            std::is_same<TSample, double>::value || BitsPerSampleOver16, double, float
          >::type LimitType;
          LimitType limit = static_cast<LimitType>(
            (std::uint32_t(1) << (this->bitsPerSample - 1)) - 1
          );

          // Now convert the samples. Negative numbers make it through this because C++ is
          // doing arithmetic shifts on the int32s and the signal reconstruction code casts
          // whole int32s to floats/doubles.
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

          // If the target data type has fewer bits, samples need to be truncated
          if constexpr(WidenFactor == -1) {

            int truncateShift = shift + this->bitsPerSample - 16;
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              if constexpr(std::is_same<TSample, std::uint8_t>::value) {
                target[sampleIndex] = static_cast<TSample>(
                  (decodedInts[sampleIndex] >> truncateShift) + 128
                );
              } else {
                target[sampleIndex] = static_cast<TSample>(
                  decodedInts[sampleIndex] >> truncateShift
                );
              }
            }
            target += sampleCount;

          } else if constexpr(WidenFactor == 1) { // Same number of bits

            // Reasoning: output types are 8, 16 or 32-bit (and 32-bit is handled separately),
            // so if we reach a match, there is no shifting required at all (see note on
            // the behavior of libwavpack a few screens up).
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              if constexpr(std::is_same<TSample, std::uint8_t>::value) {
                target[sampleIndex] = static_cast<TSample>(decodedInts[sampleIndex] + 128);
              } else {
                target[sampleIndex] = static_cast<TSample>(decodedInts[sampleIndex]);
              }
            }
            target += sampleCount;

          } else { // target integer is longer, bits need to be extended

            int topShift = 32 - this->bitsPerSample - shift;
            int repeatShift = this->bitsPerSample - 1;

            // Build a mask that will cover the bits that should be occupied after
            // the first "repeat shift" - this is needed to cut off the sign bit
            // and (since C++ does arithmetic shifts on signed types) to remove
            // the leading bits caused by sign-extending the integer.
            std::int32_t mask = (1 << (this->bitsPerSample - 1)) - 1;
            if(repeatShift > topShift) {
              mask >>= (repeatShift - topShift);
            } else {
              mask <<= (topShift - repeatShift);
            }

            // Note: instead of shifting to the top, then shifting to the repeat position,
            // we could also calculate the shift to the top (and not do it if it is zero)
            // and the shift to the repeat position from the original position.
            // Might lead to a tiny performance gain.
            for(std::size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
              std::int32_t top = decodedInts[sampleIndex] << topShift;
              std::int32_t repeat = (top >> (this->bitsPerSample - 1)) & mask;
              if constexpr(WidenFactor == 2) {
                target[sampleIndex] = static_cast<TSample>(top | repeat);
              } else { // WidenFactor can only be 3
                target[sampleIndex] = static_cast<TSample>(
                  top | repeat | (repeat >> repeatShift)
                );
              }
            } // for each sample in the chunk

            target += sampleCount;
          } // if integer needs to be truncated / extended

        } // if target type is double / integer
#pragma endregion // Convert integers to floats, doubles or integers
      } // if decoded data is float / int32

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
    throw std::runtime_error(u8"Separated channel decoding from WavPack not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack
