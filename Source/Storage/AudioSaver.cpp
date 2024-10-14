#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "Nuclex/Audio/Storage/AudioSaver.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)
#include "Opus/OpusAudioCodec.h"
#endif

#include <Nuclex/Support/Text/StringMatcher.h> // for StringMatcher

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  AudioSaver::AudioSaver() :
    codecs() {}

  // ------------------------------------------------------------------------------------------- //

  AudioSaver::~AudioSaver() {}

  // ------------------------------------------------------------------------------------------- //

  void AudioSaver::RegisterCodec(std::unique_ptr<AudioCodec> &&codec) {
    std::size_t codecCount = this->codecs.size();

    // This should be a one-liner, but clang has a nonsensical warning then typeid()
    // is called with an expression that needs to be evaluated at runtime :-(
    const AudioCodec &newCodec = *codec.get();
    const std::type_info &newType = typeid(newCodec);

    // Make sure this exact type isn't registered yet
    for(std::size_t index = 0; index < codecCount; ++index) {
      const AudioCodec &checkedCodec = *this->codecs[index].get();
      const std::type_info &existingType = typeid(checkedCodec);
      if(newType == existingType) {
        throw std::runtime_error(u8"Codec already registered");
      }
    }

    // Register the new codec into our list
    this->codecs.push_back(std::move(codec));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> AudioSaver::GetAvailableCodecs() const {
    std::vector<std::string> availableCodecNames;
    availableCodecNames.reserve(this->codecs.size());

    for(const std::unique_ptr<AudioCodec> &codec : this->codecs) {
      availableCodecNames.push_back(codec->GetName());
    }

    return availableCodecNames;
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<AudioTrackEncoderBuilder> AudioSaver::ProvideBuilder(
    const std::string &codecName
  ) const {
    using Nuclex::Support::Text::StringMatcher;

    for(std::size_t index = 0; index < this->codecs.size(); ++index) {
      constexpr bool CaseSensitive = false;
      bool matches = StringMatcher::AreEqual<CaseSensitive>(
        this->codecs[index]->GetName(), codecName
      );
      if(matches) {
        return this->codecs[index]->ProvideBuilder();
      }
    }

    // If this point is reached, we found no codec matching the provided name,
    // so let's give the user a complaint detailing what might have gone wrong.
    {
      std::string message(u8"No codec matching the name '", 28);
      message.append(codecName);
      message.append(
        u8"' can be found. Check the name for typos, for built-in codecs verify that "
        u8"they have been enabled when building Nuclex.Audio.Native, for custom codecs "
        u8"make sure that you've registered them to the AudioSaver.",
        206
      );
      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
