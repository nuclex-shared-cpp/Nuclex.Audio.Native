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

#ifndef NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H
#define NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#include "Nuclex/Audio/Storage/AudioTrackDecoder.h"
#include "./WavPackVirtualFileAdapter.h"
#include "../../Platform/WavPackApi.h"

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage { namespace WavPack {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decodes WavPack audio tracks</summary>
  class WavPackTrackDecoder : public AudioTrackDecoder {

    /// <summary>Initializes a new WavPack track decoder on the specified file</summary>
    /// <param name="file">File that will be opened and decoded</param>
    public: WavPackTrackDecoder(const std::shared_ptr<const VirtualFile> &file);
    /// <summary>Frees all resources owned by the instance</summary>
    public: ~WavPackTrackDecoder() override = default;

    /// <summary>Creates a clone of the audio track decoder</summary>
    /// <returns>A clone of the audio track decoder that can be used independently</returns>
    public: std::shared_ptr<AudioTrackDecoder> Clone() const override;

    /// <summary>Counts the number of audio channels in the track</summary>
    /// <returns>The number of audio channels in the audio track</returns>
    public: std::size_t CountChannels() const override;

    /// <summary>Retrieves the order in which channels are interleaved</summary>
    /// <returns>A list of single channels in the order they're interleaved</returns>
    public: const std::vector<ChannelPlacement> &GetChannelOrder() const override;

    /// <summary>Fetches the order of audio channels from the WavPack context</summary>
    private: void fetchChannelOrder();

    /// <summary>
    ///   Stores callbacks through which libwavpack accesses the VirtualFile instance
    /// </summary>
    /// <remarks>
    ///   When opening the WavPack file, a pointer to this structure is stored Within
    ///   the WavPackContext, so it needs to continue to exist.
    /// </remarks>
    private: ::WavpackStreamReader64 streamReader;
    /// <summary>Holds the state of the VirtualFile stream adapter</summary>
    /// <remarks>
    ///   This stores the emulated file cursor for the specific WavPack context and 
    ///   records any exceptions from the VirtualFile so they do not have to pass through
    ///   libwavpack, which is C code and won't anticipate being interrupted this way.
    /// </remarks>
    private: std::unique_ptr<ReadOnlyStreamAdapterState> state;
    /// <summary>Represents the opened WavPack file for the libwavpack API</summary>
    /// <remarks>
    ///   This ties all the internal WavPack data structure together and is passed
    ///   to any public API method in libwavpack we can call.
    /// </remarks>
    private: std::shared_ptr<::WavpackContext> context;
    /// <summary>Order in which audio channels appear</summary>
    private: std::vector<ChannelPlacement> channelOrder;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::WavPack

#endif // defined(NUCLEX_AUDIO_HAVE_WAVPACK)

#endif // NUCLEX_AUDIO_STORAGE_WAVPACK_WAVPACKTRACKDECODER_H
