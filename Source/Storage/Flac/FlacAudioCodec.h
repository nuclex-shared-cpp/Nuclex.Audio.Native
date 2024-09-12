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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACAUDIOCODEC_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACAUDIOCODEC_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/Storage/AudioCodec.h"

#include <string> // for std::string
#include <memory> // for std::unique_ptr
#include <cstdint> // for std::uint64_t

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Encodes and decodes FLAC audio files using libflac</summary>
  class FlacAudioCodec : public AudioCodec {

    /// <summary>Initializes a new audio codec</summary>
    public: FlacAudioCodec() = default;

    /// <summary>Frees all resources held by the audio codec</summary>
    public: ~FlacAudioCodec() override = default;

    /// <summary>Gives the name of the file format implemented by this codec</summary>
    /// <returns>The name of the file format this codec implements</returns>
    public: const std::string &GetName() const override;

    /// <summary>Provides commonly used file extensions for this codec</summary>
    /// <returns>The commonly used file extensions in order of preference</returns>
    public: const std::vector<std::string> &GetFileExtensions() const override;

    /// <summary>Tries to read informations for an audio container</summary>
    /// <param name="source">Source data from which the informations should be extracted</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the audio container, if the codec can load it</returns>
    public: std::optional<ContainerInfo> TryReadInfo(
      const std::shared_ptr<const VirtualFile> &source,
      const std::string &extensionHint = std::string()
    ) const override;

    /// <summary>Opens a new decoder for the specified audio file</summary>
    /// <param name="source">Source data that will be opened for audio decoding</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <param name="trackIndex">Index of the audio track to create a decoder for</param>
    /// <returns>A decoder that can be used to decode the audio track</returns>
    public: std::shared_ptr<AudioTrackDecoder> OpenDecoder(
      const std::shared_ptr<const VirtualFile> &source,
      const std::string &extensionHint = std::string(),
      std::size_t trackIndex = 0
    ) const override;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACAUDIOCODEC_H
