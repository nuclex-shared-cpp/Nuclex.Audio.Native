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

#include <string> // for std::string
#include <vector> // for std::vector
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

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class NUCLEX_AUDIO_TYPE AudioLoader {


  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOLOADER_H
