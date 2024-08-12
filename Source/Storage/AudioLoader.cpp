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

#include "Nuclex/Audio/Storage/AudioLoader.h"

#include <Nuclex/Support/Text/StringConverter.h> // for StringConverter
#include <stdexcept> // for std::runtime_error

// Also include the headers for the build-in audio codecs.
//
// This library is designed such that a new instance of the AudioLoader class will have
// all codecs the library natively understands already registered. Users of the library
// can register custom codecs or register their own implementations of the built-in
// codecs over the ones initially provided.
//
#if defined(NUCLEX_AUDIO_HAVE_FLAC)
#include "Flac/FlacAudioCodec.h"
#endif
#if defined(NUCLEX_AUDIO_HAVE_OPUS)
#include "Opus/OpusAudioCodec.h"
#endif
#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)
#include "WavPack/WavPackAudioCodec.h"
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invalid size marker for the most recent codec indices</summary>
  constexpr std::size_t InvalidIndex = std::size_t(-1);

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  AudioLoader::AudioLoader() :
    mostRecentCodecIndex(InvalidIndex),
    secondMostRecentCodecIndex(InvalidIndex) {
#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)
    RegisterCodec(std::make_unique<WavPack::WavPackAudioCodec>());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  AudioLoader::~AudioLoader() {}

  // ------------------------------------------------------------------------------------------- //

  void AudioLoader::RegisterCodec(std::unique_ptr<AudioCodec> &&codec) {
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

    const std::vector<std::string> &extensions = codec->GetFileExtensions();

    // Register the new codec into our list
    this->codecs.push_back(std::move(codec));

    // Update the extension lookup map for quick codec finding
    std::size_t extensionCount = extensions.size();
    for(std::size_t index = 0; index < extensionCount; ++index) {
      using Nuclex::Support::Text::StringConverter;

      const std::string &extension = extensions[index];
      std::string::size_type extensionLength = extension.length();

      if(extensionLength >= 1) {
        if(extension[0] == '.') {
          if(extensionLength >= 2) {
            std::string lowerExtension = StringConverter::FoldedLowercaseFromUtf8(
              extension.substr(1)
            );
            this->codecsByExtension.insert(
              ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
            );
          }
        } else {
          std::string lowerExtension = StringConverter::FoldedLowercaseFromUtf8(extension);
          this->codecsByExtension.insert(
            ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
          );
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TOutput>
  bool AudioLoader::tryCodecsInOptimalOrder(
    const std::string &extension,
    bool (*tryCodecCallback)(
      const AudioCodec &codec, const std::string &extension, TOutput &result
    ),
    TOutput &result
  ) const {
    std::size_t hintCodecIndex;

    // If an extension hint was provided, try the codec registered for the extension first
    if(extension.empty()) {
      hintCodecIndex = InvalidIndex;
    } else {
      using Nuclex::Support::Text::StringConverter;
      std::string foldedLowercaseExtension = StringConverter::FoldedLowercaseFromUtf8(extension);
      ExtensionCodecIndexMap::const_iterator iterator = (
        this->codecsByExtension.find(foldedLowercaseExtension)
      );
      if(iterator == this->codecsByExtension.end()) {
        hintCodecIndex = InvalidIndex;
      } else {
        hintCodecIndex = iterator->second;
        if(tryCodecCallback(*this->codecs[hintCodecIndex].get(), extension, result)) {
          updateMostRecentCodecIndex(hintCodecIndex);
          return true;
        }
      }
    }

    // Look up the two most recently used codecs (we don't care about race conditions here,
    // in the rare case of one occurring, we'll simple be a little less efficient and not
    // have the right codec in the MRU list...
    std::size_t mostRecent = this->mostRecentCodecIndex;
    std::size_t secondMostRecent = this->secondMostRecentCodecIndex;

    // Try the most recently used codec. It may be set to 'InvalidIndex' if this
    // is the first call to Load(). Don't try if it's the same as the extension hint.
    if((mostRecent != InvalidIndex) && (mostRecent != hintCodecIndex)) {
      if(tryCodecCallback(*this->codecs[mostRecentCodecIndex].get(), extension, result)) {
        updateMostRecentCodecIndex(mostRecent);
        return true;
      }
    }

    // Try the second most recently used codec. It, too, may be set to 'InvalidIndex'.
    // Also avoid retrying codecs we already tried.
    if(
      (secondMostRecent != InvalidIndex) &&
      (secondMostRecent != mostRecent) &&
      (secondMostRecent != hintCodecIndex)
    ) {
      if(tryCodecCallback(*this->codecs[secondMostRecent].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // Hint was not provided or wrong, most recently used codecs didn't work,
    // so go through all remaining codecs.
    std::size_t codecCount = this->codecs.size();
    for(std::size_t index = 0; index < codecCount; ++index) {
      if((index == mostRecent) || (index == secondMostRecent) || (index == hintCodecIndex)) {
        continue;
      }

      if(tryCodecCallback(*this->codecs[index].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // No codec can load the file, we give up
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void AudioLoader::updateMostRecentCodecIndex(std::size_t codecIndex) const {
    this->secondMostRecentCodecIndex.store(
      this->mostRecentCodecIndex.load(std::memory_order::memory_order_relaxed),
      std::memory_order::memory_order_relaxed
    );

    this->mostRecentCodecIndex.store(codecIndex, std::memory_order::memory_order_relaxed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage
