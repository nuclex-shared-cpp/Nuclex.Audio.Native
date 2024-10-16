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

#include "./OpusTrackEncoderBuilder.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "./OpusTrackEncoder.h"
#include "../Shared/ChannelOrderFactory.h" // for ChannelOrderFactory

#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()
#include <set> // for std::set

#include <Nuclex/Support/BitTricks.h> // for BitTricks

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  OpusTrackEncoderBuilder::OpusTrackEncoderBuilder() :
    inputChannelOrder(),
    sampleRate(),
    targetBitrate(),
    effort(1.0f) {}

  // ------------------------------------------------------------------------------------------- //

  const std::vector<
    AudioSampleFormat
  > &OpusTrackEncoderBuilder::GetSupportedSampleFormats() const {
    static const std::vector<AudioSampleFormat> supportedFormats = {
      AudioSampleFormat::Float_32
    };
    return supportedFormats;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::size_t> &OpusTrackEncoderBuilder::GetSupportedSampleRates() const {
    static const std::vector<std::size_t> supportedSampleRates; // empty vector = any
    return supportedSampleRates;
  }

  // ------------------------------------------------------------------------------------------- //

  const std::vector<std::size_t> &OpusTrackEncoderBuilder::GetPreferredSampleRates() const {
    static const std::vector<std::size_t> preferredSampleRates = {
      48000,
      24000,
      16000,
      12000,
      8000
    };
    return preferredSampleRates;
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<ChannelPlacement> OpusTrackEncoderBuilder::GetPreferredChannelOrder(
    ChannelPlacement channels
  ) const {
    std::size_t channelCount = Nuclex::Support::BitTricks::CountBits(
      static_cast<std::size_t>(channels)
    );
    std::vector<ChannelPlacement> vorbisChannelOrder = (
      Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(1, channelCount)
    );

    // Verify that this channel set is representable in an Opus audio file
    {
      std::set<ChannelPlacement> presentChannels;
      for(std::size_t index = 0; index < 17; ++index) {
        ChannelPlacement channel = static_cast<ChannelPlacement>(1 << index);
        if((channels & channel) != ChannelPlacement::Unknown) {
          presentChannels.insert(channel);
        }
      }
      for(ChannelPlacement channel : vorbisChannelOrder) {
        bool wasPresent = presentChannels.erase(channel);
        if(!wasPresent) {
          throw std::runtime_error(
            u8"Channel layout cannot be represented in Opus. The set of channels you provided "
            u8"does not fit any of the channel sets defined in the Vorbis 1 Specification, "
            u8"section 4.3.9, Output Channel Order (Opus uses the Vorbis channel layouts)."
          );
        }
      }
    }

    return vorbisChannelOrder;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &OpusTrackEncoderBuilder::SetSampleFormat(
    AudioSampleFormat format /* = AudioSampleFormat::SignedInteger_16 */
  ) {
    if(format != AudioSampleFormat::Float_32) {
      throw std::invalid_argument(u8"Opus can only store 32-bit floating point samples");
    }

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &OpusTrackEncoderBuilder::SetSampleRate(
    std::size_t samplesPerSecond /* = 48000 */
  ) {
    this->sampleRate = samplesPerSecond;
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &OpusTrackEncoderBuilder::SetChannels(
    const std::vector<ChannelPlacement> &orderedChannels
  ) {
    std::vector<ChannelPlacement> vorbisChannelOrder = (
      Shared::ChannelOrderFactory::FromVorbisFamilyAndCount(1, orderedChannels.size())
    );

    // Verify that this channel set is representable in an Opus audio file
    {
      std::set<ChannelPlacement> presentChannels;
      for(ChannelPlacement channel : orderedChannels) {
        presentChannels.insert(channel);
      }
      for(ChannelPlacement channel : vorbisChannelOrder) {
        bool wasPresent = presentChannels.erase(channel);
        if(!wasPresent) {
          throw std::runtime_error(
            u8"Channel layout cannot be represented in Opus. The set of channels you provided "
            u8"does not fit any of the channel sets defined in the Vorbis 1 Specification, "
            u8"section 4.3.9, Output Channel Order (Opus uses the Vorbis channel layouts)."
          );
        }
      }
    }

    this->inputChannelOrder = orderedChannels;
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &OpusTrackEncoderBuilder::SetTargetBitrate(
    float kilobitsPerSecond
  ) {
    this->targetBitrate = kilobitsPerSecond;
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  AudioTrackEncoderBuilder &OpusTrackEncoderBuilder::SetCompressionEffort(
    float newEffort /* = 1.0f */
  ) {
    this->effort = newEffort;
    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackEncoder> OpusTrackEncoderBuilder::Build(
    const std::shared_ptr<VirtualFile> &target
  ) {
    if(this->inputChannelOrder.empty()) {
      throw std::runtime_error(
        u8"Input channels and channel order for the encoder have not been set"
      );
    }
    if(!this->sampleRate.has_value()) {
      throw std::runtime_error(u8"Input sample rate for the encoder has not been set");
    }
    if(!this->targetBitrate.has_value()) {
      throw std::runtime_error(u8"Target birate for the encder has not been set");
    }

    std::shared_ptr<OpusTrackEncoder> encoder = std::make_shared<OpusTrackEncoder>(
      target, this->inputChannelOrder, this->sampleRate.value()
    );
    encoder->SetBitrate(this->targetBitrate.value());
    encoder->SetEffort(this->effort);

    return encoder;
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
