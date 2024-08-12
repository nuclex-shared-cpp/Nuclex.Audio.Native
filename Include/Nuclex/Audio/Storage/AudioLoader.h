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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOLOADER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOLOADER_H

#include "Nuclex/Audio/Config.h"

#include <atomic> // for std::atomic
#include <string> // for std::string
#include <vector> // for std::vector
#include <unordered_map> // for std::unordered_map
#include <memory> // for std::unique_ptr
#include <optional> // for std::optional

// I want to go with a similar design as Nuclex.Pixels.Native, but split encoding and
// decoding into different classes because the two features are so asymmetrical in
// their requirements.
//
// - The decoder needs to be /attachable/ to a track, so the track can stream more
//   data if required (-> add Autonomize() or similar method)
//
// - The encoder needs progress reporting, pause/resume and cancellation support
//
// Naming
// ------
//
// AudioAccessor
//   - Confusing?
//
// AudioDeserializer
//   - Bad. While BitmapSerializer is neat, lossy compression is not serialization
//     and it doesn't fit into the audio jargon well.
//
// AudioLoader
//   - Maybe.
//   - Perhaps this is the best choice? The term "loader" says to me that it will
//     load whole audio file, neutral about whether it's streamed or loaded whole.
//
// AudioReader
//   - Maybe.
//   - Might mislead people into thinking this is the stream decoder since reader
//     is commonly used for such things (StreamReader, FileReader, etc.).
//
// AudioStreamer
//   - Could be. Since decoding an entire file probably will be done via
//     the Autonomize() method, this could be the common decoder provider.
//     Bad because it doesn't do the streaming itself, it hands out stream...decoders?
//
// AudioDecompressor
//   - Maybe.
//   - Too close to decoder? Might give users the idea that this is doing
//     the actual decoding work rather than being the hub decoders are registered to.
//
// AudioDecoder
//   - Nope, term contained in AudioCodec, would be terribly confusing
//

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  class Track;
  class TrackInfo;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class AudioCodec;
  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class NUCLEX_AUDIO_TYPE AudioLoader {

    /// <summary>Initializes a new audio loader</summary>
    public: NUCLEX_AUDIO_API AudioLoader();

    /// <summary>Frees all resources owned by a bitmap serializer</summary>
    public: NUCLEX_AUDIO_API ~AudioLoader();

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="codec">Bitmap codec that will be registered</param>
    public: NUCLEX_AUDIO_API void RegisterCodec(std::unique_ptr<AudioCodec> &&codec);

    /// <summary>Registers a bitmap codec to load and/or save a file format</summary>
    /// <param name="TCodec">Type of bitmap codec that will be registered</param>
    public: template<typename TCodec>
    void RegisterCodec() {
      std::unique_ptr<AudioCodec> codec = std::make_unique<TCodec>();
      RegisterCodec(std::move(codec));
    }

    /// <summary>Tries to read informations about a bitmap</summary>
    /// <param name="file">File from which informations will be read</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the bitmap, if it is a supported format</returns>
    public: NUCLEX_AUDIO_API std::optional<TrackInfo> TryReadInfo(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Tries to read informations about a bitmap</summary>
    /// <param name="path">Path of the file informations will be read from</param>
    /// <returns>Informations about the bitmap, if it is a supported format</returns>
    public: NUCLEX_AUDIO_API std::optional<TrackInfo> TryReadInfo(const std::string &path) const;
#if 0
    /// <summary>Checks whether the bitmap store can load the specified file</summary>
    /// <param name="file">File the bitmap store will check</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>True if the bitmap store thinks it can load the file</returns>
    public: NUCLEX_AUDIO_API bool CanLoad(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Checks whether the bitmap store can load the specified file</summary>
    /// <param name="path">Path of the file the bitmap store will check</param>
    /// <returns>True if the bitmap store thinks it can load the file</returns>
    public: NUCLEX_AUDIO_API bool CanLoad(const std::string &path) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="file">File the bitmap store will load</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>The bitmap loaded from the specified file</returns>
    public: NUCLEX_AUDIO_API Track Load(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Loads the specified file into a new Bitmap</summary>
    /// <param name="path">Path of the file the bitmap store will load</param>
    /// <returns>The bitmap loaded from the specified file</returns>
    public: NUCLEX_AUDIO_API Track Load(const std::string &path) const;
#endif
    /// <summary>Builds a new iterator that checks codecs in the most likely order</summary>
    /// <param name="extension">File extension, if known</param>
    /// <param name="tryCodecCallback">
    ///   Address of a method that will be called to try each registered codec
    /// </param>
    /// <returns>True as soon as one codec reports success, false if none did</returns>
    /// <remarks>
    ///   This is only a template so I don't have do expose the iterator implementation
    ///   in a public header. There's exactly one specialization of the method.
    /// </remarks>
    private: template<typename TOutput>
    bool tryCodecsInOptimalOrder(
      const std::string &extension,
      bool (*tryCodecCallback)(
        const AudioCodec &codec, const std::string &extension, TOutput &result
      ),
      TOutput &result
    ) const;

    /// <summary>Updates the list of most recently used codecs</summary>
    /// <param name="codecIndex">Index of the codec that was most recently used</param>
    private: void updateMostRecentCodecIndex(std::size_t codecIndex) const;

    /// <summary>Maps file extensions to codec indices</summary>
    private: typedef std::unordered_map<std::string, std::size_t> ExtensionCodecIndexMap;
    /// <summary>Stores a sequential list of codecs</summary>
    private: typedef std::vector<std::unique_ptr<AudioCodec>> CodecVector;

    /// <summary>Allows the bitmap store to look up a codec by its file extension</summary>
    /// <remarks>
    ///   Extensions are stored in UTF-8 folded lowercase for case insensitivity.
    /// </remarks>
    private: ExtensionCodecIndexMap codecsByExtension;
    /// <summary>Codecs that have been registered with the bitmap store</summary>
    private: CodecVector codecs;
    /// <summary>Codec that was most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> mostRecentCodecIndex;
    /// <summary>Codec that was second-most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> secondMostRecentCodecIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOLOADER_H
