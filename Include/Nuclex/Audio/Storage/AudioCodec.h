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

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Codec that loads and saves bitmaps in a predefined file format</summary>
  class NUCLEX_AUDIO_TYPE AudioCodec {

    /// <summary>Frees all resources owned by the instance</summary>
    public: virtual ~AudioCodec() = default;

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

    // Probably sensible to use std::shared_ptr<VirtualFile> for these,
    // so the decoder/streamer/encoder/whatever can hold onto the virtual file.

    // CreateDecoder
    // CreateDecodingContext
    // CreateStreamer
    // CreateStreamDecoder
    // CreateEncoder
    // CreateEncodingContext

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOCODEC_H
