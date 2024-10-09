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
  #error This file must be included from WaveformReader.cpp directly
#endif

// Shouldn't be an issue including these here, we're outside any namespaces
#include "Nuclex/Audio/Processing/Quantization.h"
#include "Nuclex/Audio/Processing/Reconstruction.h"
#include "Nuclex/Audio/Processing/BitExtension.h"

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Waveform {

  // ------------------------------------------------------------------------------------------- //

  template<
    typename TSample,
    bool BitsPerSampleOver16 /* = false */,
    int WidenFactor /* = 0 */
  >
  void WaveformReader::readInterleavedAndConvert(
    TSample *target, std::uint64_t startFrame, std::size_t frameCount
  ) {
    constexpr bool storedSamplesAreFloat = (WidenFactor < 0); // per convention
    constexpr bool targetIntegerHasFewerBits = (WidenFactor == 0);
    constexpr bool decodedIntegerMatchesTargetBits = (WidenFactor == 1);
    constexpr bool targetTypeIsFloat = (
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value
    );

    // Allocate an intermedia buffer. We use std::byte because we're going to be
    // reading into it without knowing the actual data type in the file at compile time
    std::size_t readChunkSize = frameCount;
    while(readChunkSize >= 12000) {
      readChunkSize >>= 1;
    }
    std::vector<std::byte> readBuffer(readChunkSize * this->bytesPerFrame);

    while(0 < frameCount) {

      // If this is the last chunk or if the user requested less frames than the chunk
      // size in the first place, limit the amount of data we request from libwavpack to
      // the number of frames we actually need.
      std::size_t readFrameCount;
      if(frameCount < readChunkSize) {
        readFrameCount = frameCount;
      } else {
        readFrameCount = readChunkSize;
      }

      // Read into our intermediate buffer.
      this->file->ReadAt(
        startFrame * this->bytesPerFrame + this->firstSampleOffset,
        readFrameCount * this->bytesPerFrame,
        readBuffer.data()
      );
      startFrame += readFrameCount;

      std::size_t readSampleCount = readFrameCount * this->trackInfo.ChannelCount;

      if constexpr(storedSamplesAreFloat) {
        typedef typename std::conditional<
          WidenFactor == -1, float, double // -1 indicates float, -2 indicates double
        >::type StoredFloatType;
        StoredFloatType *decodedFloats = reinterpret_cast<StoredFloatType *>(readBuffer.data());

        if constexpr(targetTypeIsFloat) {

          // Whether target is float or double -> just cast the floats
          for(std::size_t sampleIndex = 0; sampleIndex < readSampleCount; ++sampleIndex) {
            target[sampleIndex] = static_cast<double>(decodedFloats[sampleIndex]);
          }
          target += readSampleCount;

        } else { // if target type is ^^ floating point ^^ / vv integer vv

          // Determine the 'limit' type to use for the numeric range in the output type.
          // This is about the target data type, so we don't use 'BitsPerSampleOver16' here.
          typedef typename std::conditional<
            sizeof(TSample) < 17, float, double
          >::type LimitType;
          constexpr LimitType limit = static_cast<LimitType>(
            (std::uint32_t(1) << (sizeof(TSample) * 8 - 1)) - 1
          );

          while(3 < readSampleCount) {
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
            readSampleCount -= 4;
          }
          while(0 < readSampleCount) {
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
            --readSampleCount;
          }
        } // if target type is double / integer

      } else { // if decoded data from libwavpack is ^^ float ^^ / vv int32 vv
#if 0
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
        int shift = static_cast<int>(this->bytesPerSample * 8 - this->bitsPerSample);

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
            Nuclex::Audio::Processing::Reconstruction::ShiftAndDivideInt32ToFloatx4(
              decodedInts, shift, limit, target
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
          if constexpr(targetIntegerHasFewerBits) {

            int truncateShift = static_cast<int>(shift + this->bitsPerSample - 16);
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

          } else if constexpr(decodedIntegerMatchesTargetBits) { // Same number of bits

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

            int topShift = static_cast<int>(32 - this->bitsPerSample - shift);
            int repeatShift = static_cast<int>(this->bitsPerSample - 1);

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
            while(3 < sampleCount) {
              std::int32_t converted[4];
              if constexpr(WidenFactor == 2) {
                Processing::BitExtension::ShiftAndRepeatSignedx4(
                  topShift, decodedInts, repeatShift, mask, converted
                );
              } else {
                Processing::BitExtension::ShiftAndTripleSignedx4(
                  topShift, decodedInts, repeatShift, mask, converted
                );
              }

              if constexpr(std::is_same<TSample, std::uint8_t>::value) {
                target[0] = static_cast<TSample>(converted[0] + 128);
                target[1] = static_cast<TSample>(converted[1] + 128);
                target[2] = static_cast<TSample>(converted[2] + 128);
                target[3] = static_cast<TSample>(converted[3] + 128);
              } else {
                target[0] = static_cast<TSample>(converted[0]);
                target[1] = static_cast<TSample>(converted[1]);
                target[2] = static_cast<TSample>(converted[2]);
                target[3] = static_cast<TSample>(converted[3]);
              }
              decodedInts += 4;
              target += 4;
              sampleCount -= 4;
            }
            while(0 < sampleCount) {
              std::int32_t converted;
              if constexpr(WidenFactor == 2) {
                converted = Processing::BitExtension::ShiftAndRepeatSigned(
                  topShift, decodedInts[0], repeatShift, mask
                );
              } else {
                converted = Processing::BitExtension::ShiftAndTripleSigned(
                  topShift, decodedInts[0], repeatShift, mask
                );
              }

              if constexpr(std::is_same<TSample, std::uint8_t>::value) {
                target[0] = static_cast<TSample>(converted + 128);
              } else {
                target[0] = static_cast<TSample>(converted);
              }
              ++decodedInts;
              ++target;
              --sampleCount;
            }

          } // if integer needs to be truncated / extended

        } // if target type is double / integer
#endif
      } // if decoded data is float / int32

      frameCount -= readFrameCount;

    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<
    typename TSample,
    bool BitsPerSampleOver16 /* = false */,
    int WidenFactor /* = 0 */
  >
  void WaveformReader::readInterleavedConvertAndSeparate(
    TSample *targets[], std::uint64_t startFrame, std::size_t frameCount
  ) {
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Waveform
