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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOCODEC_H
#define NUCLEX_AUDIO_STORAGE_AUDIOCODEC_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ContainerInfo.h"
#include "Nuclex/Audio/TrackInfo.h"

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;
  class AudioTrackDecoder;
  class AudioTrackEncoderBuilder;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Codec that loads and saves bitmaps in a predefined file format</summary>
  class NUCLEX_AUDIO_TYPE AudioCodec {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioCodec() = default;

    /// <summary>Gives the name of the file format implemented by this codec</summary>
    /// <returns>The name of the file format this codec implements</returns>
    public: virtual const std::string &GetName() const = 0;

    /// <summary>Provides commonly used file extensions for this codec</summary>
    /// <returns>The commonly used file extensions in order of preference</returns>
    public: virtual const std::vector<std::string> &GetFileExtensions() const = 0;

    /// <summary>Tries to read informations for an audio container</summary>
    /// <param name="source">Source data from which the informations should be extracted</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <returns>Informations about the audio container, if the codec can load it</returns>
    public: virtual std::optional<ContainerInfo> TryReadInfo(
      const std::shared_ptr<const VirtualFile> &source,
      const std::string &extensionHint = std::string()
    ) const = 0;

    /// <summary>Opens a new decoder for the specified audio file</summary>
    /// <param name="source">Source data that will be opened for audio decoding</param>
    /// <param name="extensionHint">Optional file extension the loaded data had</param>
    /// <param name="trackIndex">Index of the audio track to create a decoder for</param>
    /// <returns>A decoder that can be used to decode the audio track</returns>
    public: virtual std::shared_ptr<AudioTrackDecoder> TryOpenDecoder(
      const std::shared_ptr<const VirtualFile> &source,
      const std::string &extensionHint = std::string(),
      std::size_t trackIndex = 0
    ) const = 0;

    // TryOpenDecoder(ContainerStreamReader)
    //
    //   Might add something like this. Unless it is always as "simple" as deinterleaving
    //   the audio data from a container and treating that as a VirtualFile, but containers
    //   might not have easy ways to report the ultimate size of an isolated stream.
    //
    //   For now, Nuclex.Audio.Native doesn't worry about complex media containers yet
    //

    /// <summary>Reports whether this codec can be encoded to</summary>
    /// <returns>True if the codec can provide encoders, false if it decodes only</returns>
    /// <remarks>
    ///   The default implementation for this always returns false. If you implement a codec
    ///   that only decodes audio, you can simply leave this method out.
    /// </remarks>
    public: NUCLEX_AUDIO_API virtual bool CanEncode() const;

    /// <summary>
    ///   Requests a builder through which encoders for this codec can be configured and
    ///   then created
    /// </summary>
    /// <returns>The encoder builder for this codec</returns>
    /// <remarks>
    ///   The default implementation throws an exception that reports that this codec
    ///   does not support encoding. If you implement a codec that only decodes audio,
    ///   you can leave this method out and let the default implementation do its thing.
    /// </remarks>
    public: NUCLEX_AUDIO_API virtual std::shared_ptr<
      AudioTrackEncoderBuilder
    > ProvideBuilder() const;

    // CreateEncoder()
    // CreateEncodingContext()
    //
    //   May add these to allow for encoding audio
    //

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOCODEC_H
