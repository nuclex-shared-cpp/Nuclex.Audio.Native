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

#include "./OpusTrackEncoder.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/Processing/SampleConverter.h"
#include "../Shared/ChannelOrderFactory.h"
#include "../../Platform/OpusEncoderApi.h"

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  OpusTrackEncoder::OpusTrackEncoder(
    const std::shared_ptr<VirtualFile> &target,
    const std::vector<ChannelPlacement> &inputChannelOrder,
    std::size_t sampleRate
  ) :
    inputChannelOrder(inputChannelOrder),
    isVorbisChannelOrder(false),
    encoderCallbacks(),
    state(),
    opusComments(),
    opusEncoder() {

    this->state = FileAdapterFactory::CreateAdapterForWriting(
      target, this->encoderCallbacks
    );

    this->opusComments = Platform::OpusEncoderApi::CreateComments();

    this->opusEncoder = Platform::OpusEncoderApi::CreateFromCallbacks(
      this->state.get(),
      &this->encoderCallbacks,
      sampleRate,
      this->inputChannelOrder.size(),
      this->opusComments
    );

    // This decides the latency and complexity the encoder uses. Since this library
    // doesn't care about streaming, we pick 'Audio' for best quality.
    Platform::OpusEncoderApi::ControlEncoder(
      this->opusEncoder, OPUS_SET_APPLICATION_REQUEST, OPUS_APPLICATION_AUDIO
    );

    // Use 48000 Hz, maximum bandwidth for Opus. 
    Platform::OpusEncoderApi::ControlEncoder(
      this->opusEncoder, OPUS_SET_BANDWIDTH_REQUEST, OPUS_BANDWIDTH_FULLBAND
    );

    #if 0
    // I assume this always picks CELT (Opus is a hybrid encoder combining SILK for
    // speech and CELT for more coplex audio). The encoder should be able to detect
    // the correct type from packet to packet, but there are some reports of sudden
    // audible quality changes when it switches, so for high-bandwidth, high-quality
    // encoding, forcing CELT only might be sensible? Unclear.
    Platform::OpusEncoderApi::ControlEncoder(
      this->opusEncoder, OPUS_SET_SIGNAL_REQUEST, OPUS_SIGNAL_MUSIC
    );
    #endif

    // Finally, check if the channel order matches the Vorbis channel order.
    // If it's identical, it means we can feed interleaved float samples directly
    // to the Opus encoder without having to re-weave the channels.
    std::size_t channelCount = this->inputChannelOrder.size();
    std::vector<ChannelPlacement> vorbisChannelOrder = (
      Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(1, channelCount)
    );
    this->isVorbisChannelOrder = (this->inputChannelOrder == vorbisChannelOrder);
  }

  // ------------------------------------------------------------------------------------------- //

  OpusTrackEncoder::~OpusTrackEncoder() {
    this->opusEncoder.reset();
    this->opusComments.reset();
    this->state.reset();
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::SetBitrate(float kilobitsPerSecond) {
    Platform::OpusEncoderApi::ControlEncoder(
      this->opusEncoder,
      OPUS_SET_BITRATE_REQUEST, static_cast<int>(kilobitsPerSecond * 1000.0f)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::SetEffort(float effort) {
    int effortInt = static_cast<int>(effort * 10.0f + 0.5f);
    effortInt = std::min(std::max(effortInt, 0), 10);

    Platform::OpusEncoderApi::ControlEncoder(
      this->opusEncoder,
      OPUS_SET_COMPLEXITY_REQUEST, effortInt
    );
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<ChannelPlacement> &OpusTrackEncoder::GetChannelOrder() const {
    return this->inputChannelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::Flush() {
    Platform::OpusEncoderApi::Drain(this->opusEncoder);
    FileAdapterState::RethrowPotentialException(*state);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeInterleavedUint8(
    const std::uint8_t *buffer, std::size_t frameCount
  ) {
    encodeInterleaved(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeInterleavedInt16(
    const std::int16_t *buffer, std::size_t frameCount
  ) {
    encodeInterleaved(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeInterleavedInt32(
    const std::int32_t *buffer, std::size_t frameCount
  ) {
    encodeInterleaved(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeInterleavedFloat(
    const float *buffer, std::size_t frameCount
  ) {
    if(this->isVorbisChannelOrder) {
      Platform::OpusEncoderApi::WriteFloats(
        this->opusEncoder, buffer, frameCount
      );
      FileAdapterState::RethrowPotentialException(*state);
    } else {
      encodeInterleaved(buffer, frameCount);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeInterleavedDouble(
    const double *buffer, std::size_t frameCount
  ) {
    encodeInterleaved(buffer, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeSeparatedUint8(
    const std::uint8_t *buffers[], std::size_t frameCount
  ) {
    encodeSeparated(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeSeparatedInt16(
    const std::int16_t *buffers[], std::size_t frameCount
  ) {
    encodeSeparated(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeSeparatedInt32(
    const std::int32_t *buffers[], std::size_t frameCount
  ) {
    encodeSeparated(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeSeparatedFloat(
    const float *buffers[], std::size_t frameCount
  ) {
    encodeSeparated(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void OpusTrackEncoder::EncodeSeparatedDouble(
    const double *buffers[], std::size_t frameCount
  ) {
    encodeSeparated(buffers, frameCount);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void OpusTrackEncoder::encodeInterleaved(
    const TSample *buffer, std::size_t frameCount
  ) {
    constexpr bool samplesAreFloatingPoint = (
      std::is_same<TSample, float>::value ||
      std::is_same<TSample, double>::value
    );
    if constexpr(samplesAreFloatingPoint) {
      //std::vector<float> convertedSamples(frameCount * this->inputChannelOrder.size()) {
      //}
    }

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TSample>
  void OpusTrackEncoder::encodeSeparated(
    const TSample *buffers[], std::size_t frameCount
  ) {
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
