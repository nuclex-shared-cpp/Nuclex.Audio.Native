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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOSAVER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOSAVER_H

#include "Nuclex/Audio/Config.h"

#include <unordered_map> // for std::unordered_map
#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

// Naming
// ------
//
// I'm not very fond of naming this thing 'AudioSaver', it just sounds flat and
// somehow silly. But just like with the 'AudioLoader' the term serialization
// doesn't fit and calling these classes 'AudioEncoder' and 'AudioDecoder' would
// have have their names collide with the codecs and their encoder/decoder interfaces
// which are also exposed to the user of the library.
//
// Thus, until I find an actually decent term that isn't confusing, 'AudioSaver' it is.
//

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class AudioCodec;
  class AudioTrackEncoderBuilder;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a set of audio codecs and uses them to encode audio files</summary>
  /// <remarks>
  ///   This class mirrors the <see cref="AudioLoader" />, but manages codecs that can
  ///   be used to encode audio files. Whereas the <see cref="AudioLoader" /> identifies
  ///   file types and uses the correct codec, this one gives you the choice of which
  ///   codec you wish to use to encode your audio data.
  /// </remarks>
  class NUCLEX_AUDIO_TYPE AudioSaver {

    /// <summary>Initializes a new audio saver</summary>
    public: NUCLEX_AUDIO_API AudioSaver();

    /// <summary>Frees all resources owned by an audio saver</summary>
    public: NUCLEX_AUDIO_API ~AudioSaver();

    /// <summary>Registers an audio codec to save a file format</summary>
    /// <param name="codec">Audio codec that will be registered</param>
    public: NUCLEX_AUDIO_API void RegisterCodec(std::unique_ptr<AudioCodec> &&codec);

    /// <summary>Registers an audio codec to save a file format</summary>
    /// <param name="TCodec">Type of audio codec that will be registered</param>
    public: template<typename TCodec>
    void RegisterCodec() {
      std::unique_ptr<AudioCodec> codec = std::make_unique<TCodec>();
      RegisterCodec(std::move(codec));
    }

    /// <summary>Provides a list of the names of all registered audio codecs</summary>
    /// <returns>A list containing the names of all usable audio codecs</returns>
    public: NUCLEX_AUDIO_API std::vector<std::string> GetAvailableCodecs() const;

    /// <summary>
    ///   Provides an &quot;encoder builder&quot; that lets you configure encoding options
    ///   for the chosen codec and construct the actual encoder
    /// </summary>
    /// <param name="codecName">
    ///   Name of the audio codec for which an encoder builder will be provided
    /// </param>
    public: NUCLEX_AUDIO_API std::shared_ptr<AudioTrackEncoderBuilder> ProvideBuilder(
      const std::string &codecName
    ) const;

    /// <summary>Maps file extensions to codec indices</summary>
    private: typedef std::unordered_map<std::string, std::size_t> ExtensionCodecIndexMap;
    /// <summary>Stores a sequential list of codecs</summary>
    private: typedef std::vector<std::unique_ptr<AudioCodec>> CodecVector;

    /// <summary>Allows the audio loader to look up a codec by its file extension</summary>
    /// <remarks>
    ///   Extensions are stored in UTF-8 folded lowercase for case insensitivity.
    /// </remarks>
    private: ExtensionCodecIndexMap codecsByExtension;
    /// <summary>Codecs that have been registered with the audio loader</summary>
    private: CodecVector codecs;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOSAVER_H
