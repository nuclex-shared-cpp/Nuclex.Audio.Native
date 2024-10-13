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

#include "Nuclex/Audio/ContainerInfo.h"

#include <atomic> // for std::atomic
#include <unordered_map> // for std::unordered_map
#include <memory> // for std::unique_ptr

// Naming
// ------
//
// AudioLoader
//   - Maybe.
//   - Perhaps this is the best choice? The term "loader" says to me that it will
//     load whole audio files, neutral about whether streamed or not.
//
// AudioCodecManager
//   - Does *using* the codecs by forwarding calls to then count as managing?
//
// AudioCodecRepository
//   - This is relatively neutral
//   - Somehow, I've got less scruples about forwarding calls to codecs in this
//     than with 'AudioCodecManager.'
//
//
// Naming - probably not
// ---------------------
//
// AudioAccessor
//   - Confusing?
//
// AudioDeserializer
//   - Bad. While BitmapSerializer is neat, lossy compression is not serialization
//     and it doesn't fit into the audio jargon well.
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

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class AudioCodec;
  class VirtualFile;
  class AudioTrackDecoder;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a set of audio codecs and uses them to decode audio files</summary>
  /// <remarks>
  ///   <para>
  ///     The actual workings of this class are rather simple. It allows you to register
  ///     some codecs to it and when you try to read an audio file, they will be queried in
  ///     the classic chain-of-responsibility design pttern.
  ///   </para>
  ///   <para>
  ///     Except that there's a bit of optimization going on. If you provide a file extension
  ///     hint, that will be used to try certain codecs first. It also anticipates that your
  ///     application or user has likely settled on one or two audio formats, so the two most
  ///     recently used codecs will be tried first, too.
  ///   </para>
  ///   <para>
  ///     The audio loader starts out with all codecs that had been enabled for the library
  ///     build already registered. You can register additional codecs at any time and you
  ///     are free to register your own implementations of existing codecs over the built-in
  ///     ones, replacing them as defaults for the respective file extensions.
  ///   </para>
  /// </remarks>
  class NUCLEX_AUDIO_TYPE AudioLoader {

    /// <summary>Initializes a new audio loader</summary>
    public: NUCLEX_AUDIO_API AudioLoader();

    /// <summary>Frees all resources owned by an audio loader</summary>
    public: NUCLEX_AUDIO_API ~AudioLoader();

    /// <summary>Registers an audio codec to load a file format</summary>
    /// <param name="codec">Audio codec that will be registered</param>
    public: NUCLEX_AUDIO_API void RegisterCodec(std::unique_ptr<AudioCodec> &&codec);

    /// <summary>Registers an audio codec to load a file format</summary>
    /// <param name="TCodec">Type of audio codec that will be registered</param>
    public: template<typename TCodec>
    void RegisterCodec() {
      std::unique_ptr<AudioCodec> codec = std::make_unique<TCodec>();
      RegisterCodec(std::move(codec));
    }

    /// <summary>Tries to read informations about an audio file</summary>
    /// <param name="file">File from which informations will be read</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the audio file, if it is a supported format</returns>
    public: NUCLEX_AUDIO_API std::optional<ContainerInfo> TryReadInfo(
      const std::shared_ptr<const VirtualFile> &file,
      const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Tries to read informations about a audio file</summary>
    /// <param name="path">Path of the file informations will be read from</param>
    /// <returns>Informations about the audio file, if it is a supported format</returns>
    public: NUCLEX_AUDIO_API std::optional<ContainerInfo> TryReadInfo(
      const std::string &path
    ) const;

#if 0
    /// <summary>Checks whether the audio loader can load the specified file</summary>
    /// <param name="file">File the audio loader will check</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>True if the audio loader thinks it can load the file</returns>
    public: NUCLEX_AUDIO_API bool CanLoad(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Checks whether the adio loader can load the specified file</summary>
    /// <param name="path">Path of the file the audio loader will check</param>
    /// <returns>True if the audio loader thinks it can load the file</returns>
    public: NUCLEX_AUDIO_API bool CanLoad(const std::string &path) const;

    /// <summary>Loads the specified file into a new audio track</summary>
    /// <param name="file">File the audio loader will load</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <returns>The audio track loaded from the specified file</returns>
    public: NUCLEX_AUDIO_API Track Load(
      const VirtualFile &file, const std::string &extensionHint = std::string()
    ) const;

    /// <summary>Loads the specified file into a new audio track</summary>
    /// <param name="path">Path of the file the audio loader  will load</param>
    /// <returns>The audio track loaded from the specified file</returns>
    public: NUCLEX_AUDIO_API Track Load(const std::string &path) const;
#endif

    /// <summary>Creates a low-level track decoder for the specified audio file</summary>
    /// <param name="file">File the track decoder will access</param>
    /// <param name="extensionHint">
    ///   Optional file extension to help detection (may speed things up)
    /// </param>
    /// <param name="trackIndex">
    ///   Index of the audio track that will be accessed (this is not the channel,
    ///   rather, it's meant for containers such as .mp4 or .mkv that can contain
    ///   multiple audio tracks, i.e. for different languages
    /// </param>
    /// <returns>
    ///   An audio track decoder through samples can be read from the audio file
    /// </returns>
    public: NUCLEX_AUDIO_API std::shared_ptr<AudioTrackDecoder> OpenDecoder(
      const std::shared_ptr<const VirtualFile> &file,
      const std::string &extensionHint = std::string(),
      std::size_t trackIndex = 0
    ) const;

    /// <summary>Creates a low-level track decoder for the specified audio file</summary>
    /// <param name="path">Path of the file the track decoder will access</param>
    /// <param name="trackIndex">
    ///   Index of the audio track that will be accessed (this is not the channel,
    ///   rather, it's meant for containers such as .mp4 or .mkv that can contain
    ///   multiple audio tracks, i.e. for different languages
    /// </param>
    /// <returns>
    ///   An audio track decoder through samples can be read from the audio file
    /// </returns>
    public: NUCLEX_AUDIO_API std::shared_ptr<AudioTrackDecoder> OpenDecoder(
      const std::string &path,
      std::size_t trackIndex = 0
    ) const;

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

    /// <summary>Allows the audio loader to look up a codec by its file extension</summary>
    /// <remarks>
    ///   Extensions are stored in UTF-8 folded lowercase for case insensitivity.
    /// </remarks>
    private: ExtensionCodecIndexMap codecsByExtension;
    /// <summary>Codecs that have been registered with the audio loader</summary>
    private: CodecVector codecs;
    /// <summary>Codec that was most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> mostRecentCodecIndex;
    /// <summary>Codec that was second-most recently accessed, -1 if none</summary>
    private: mutable std::atomic<std::size_t> secondMostRecentCodecIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOLOADER_H
