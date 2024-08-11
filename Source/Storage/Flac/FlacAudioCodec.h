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

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACAUDIOCODEC_H
