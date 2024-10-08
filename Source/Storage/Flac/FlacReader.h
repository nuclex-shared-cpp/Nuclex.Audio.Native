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

#ifndef NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H
#define NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H

#include "Nuclex/Audio/Config.h"

#if defined(NUCLEX_AUDIO_HAVE_FLAC)

#include "Nuclex/Audio/AudioSampleFormat.h"
#include "Nuclex/Audio/ChannelPlacement.h"

#include "./FlacVirtualFileAdapter.h" // for the FlacDecodeProcessor interface

#include <optional> // for std::optional

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  struct TrackInfo;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  struct ReadOnlyFileAdapterState;

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

namespace Nuclex { namespace Audio { namespace Storage { namespace Flac {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Utility class with intermediate methods used to decode FLAC files</summary>
  /// <remarks>
  ///   <para>
  ///     This is a high-level wrapper to libflac that takes care of most of the plumbing
  ///     required to integrate libflac into a C++ environment with exceptions. Because libflac
  ///     uses file cursors (or rather, sample/frame cursors), this class is designed to be used
  ///     from a single thread at a time.
  ///   </para>
  ///   <para>
  ///     The &quot;FlacDecodeProcessor&quot; this class inherits from is part of the
  ///     &quot;FlacVirtualFileAdapter&quot;. Due to the design of libflac, the callbacks
  ///     to receive metadata and decoded audio samples must be set when opening a file,
  ///     so the virtual file adapter is forced to ignore &quot;separation of concerns&quot;
  ///     here and handle the decoding callbacks on the virtual file adapter level.
  ///   </para>
  /// </remarks>
  class FlacReader : protected FlacDecodeProcessor {

    /// <summary>Signature for the callback function used to process decoded samples</summary>
    /// <param name="userPointer">Pointer passed, unchanged, from the decode method</param>
    /// <param name="buffers">Buffers containing the separated audio channels</param>
    /// <param name="frameCount">Total number of samples in each of the channels</param>
    public: typedef void ProcessDecodedSamplesFunction(
      void *userPointer, const std::int32_t *const buffers[], std::size_t frameCount
    );

    /// <summary>Determines the native sample format from Flac's parameters</summary>
    /// <param name="bitsPerSample">The number of valid bits in each sample</param>
    /// <returns>The equivalent sample format enumeration value</returns>
    public: static AudioSampleFormat SampleFormatFromBitsPerSample(int bitsPerSample);

    /// <summary>
    ///   Determines the channel placement from the channel count and assignment
    /// </summary>
    /// <param name="channelCount">Number of audio channels in the FLAC file</param>
    /// <param name="channelAssignments">Channel assignment set that is used</param>
    /// <returns>The equivalent ChannelPlacement flag combination</returns>
    /// <remarks>
    ///   FLAC defines standard channel layouts for each possible number of channels up
    ///   to 8 (IETF draft FLAC specification 9.1.3). These can be overriden via a Vorbis
    ///   comment tag, but so long as the channel layout being encoded matches the standard
    ///   layout, said tag will be omitted. This method will return the channel placements
    ///   given the number of channels and their channel assignment bits.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromChannelCountAndAssignment(
      std::size_t channelCount, ::FLAC__ChannelAssignment channelAssignments
    );

    /// <summary>Parses the channel placement from the Vorbis comment tag</summary>
    /// <param name="channelMaskValue">Channel mask assigned via a Vorbis comment tag</param>
    /// <returns>The channel placement parsed from the Vorbis comment tag</returns>
    /// <remarks>
    ///   If the channel layout in a FLAC file deviates from the standard layout defined
    ///   per channel count, a Vorbis comment assigning the channel layout can be added as
    ///   Vorbis comment with the key <code>WAVEFORMATEXTENSIBLE_CHANNEL_MASK</code> and
    ///   matching Microsoft's channel mask bits from the Waveform file format
    ///   (IETF draft FLAC specification 8.6.2). These will match out ChannelPlacement
    ///   enum exactly, so, if present, the channel placements from this tag should be used.
    /// </remarks>
    public: static ChannelPlacement ChannelPlacementFromWaveFormatExtensibleTag(
      const std::string_view &channelMaskValue
    );

    /// <summary>Initializes a new Flac reader accessing the specified file</summary>
    /// <param name="file">File that will be opened by the reader</param>
    public: FlacReader(const std::shared_ptr<const VirtualFile> &file);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~FlacReader();

    /// <summary>Reads the FLAC file's metadata</summary>
    /// <param name="target">Track informatio container that will receive the metadata</param>
    public: void ReadMetadata(TrackInfo &target);

    /// <summary>Counts the total number of frames (= samples in each channel)</summary>
    /// <returns>The total number of frames in the audio file</returns>
    public: std::uint64_t CountTotalFrames() const;

    /// <summary>Retrieves the current position of the frame cursor</summary>
    /// <returns>The frame cursor, pointing at the frame that will be decoded next</returns>
    public: std::uint64_t GetFrameCursorPosition() const;

    /// <summary>Seeks to the specified frame</summary>
    /// <param name="frameIndex">Index of the frame (= sample index on all channels)</param>
    public: void Seek(std::uint64_t frameIndex);

    /// <summary>Decodes the requested number of samples</summary>
    /// <param name="buffers">Buffers into which the samples will be decoded</param>
    /// <param name="processDecodedSamples">
    ///   Callback through which each decoded block of samples will be delivered
    /// </param>
    /// <param name="frameCount">Number of frames (= samples on all channels) to decode</param>
    public: void DecodeSeparated(
      void *userPointer,
      ProcessDecodedSamplesFunction processDecodedSamples,
      std::size_t frameCount
    );

    /// <summary>Called to process any metadata encountered in the FLAC file</summary>
    /// <param name="metadata">Metadata the FLAC stream decoder has encountered</param>
    protected: void ProcessMetadata(
      const ::FLAC__StreamMetadata &metadata
    ) noexcept override;

    /// <summary>Called to process an audio frame after it has been decoded</summary>
    /// <param name="frame">Informations about the decoded audio frame</param>
    /// <param name="buffer">Stores the decoded audio samples</param>
    /// <returns>True to continue decoding, false to stop at this point</returns>
    protected: bool ProcessAudioFrame(
      const ::FLAC__Frame &frame,
      const ::FLAC__int32 *const buffer[]
    ) override;

    /// <summary>Called to provide a detailed status when a decoding error occurs</summary>
    /// <param name="status">Error status of the stream decoder</param>
    protected: void HandleError(
      ::FLAC__StreamDecoderErrorStatus status
    ) noexcept override;

    /// <summary>Processes a StreamInfo block encountered in the FLAC file</summary>
    /// <param name="streamInfo">StreamInfo block describing the file's properties</param>
    private: void processStreamInfo(
      const ::FLAC__StreamMetadata_StreamInfo &streamInfo
    ) noexcept;

    /// <summary>Processes a Vorbis comment block encountered in the FLAC file</summary>
    /// <param name="vorbisComment">Vorbis comment block containing individual entries</param>
    private: void processVorbisComment(
      const ::FLAC__StreamMetadata_VorbisComment &vorbisComment
    ) noexcept;

    /// <summary>File the reader is accessing</summary>
    private: std::shared_ptr<const VirtualFile> file;
    /// <summary>State (emulated file cursor, errors) of the virtual file adapter</summary>
    private: std::unique_ptr<ReadOnlyFileAdapterState> state;
    /// <summary>Stream decoder from libflac that does the actual decoding work</summary>
    private: std::shared_ptr<::FLAC__StreamDecoder> streamDecoder;
    /// <summary>Potential error reported through libflac's error callback</summary>
    private: std::exception_ptr error;

    /// <summary>Whether the metadata block was delivered</summary>
    private: bool obtainedMetadata;
    /// <summary>Whether a Vorbis comment block with a channel mask was delivered</summary>
    private: bool obtainedChannelMask;
    /// <summary>Channel assignment in the most recently decoded frame</summary>
    private: std::optional<::FLAC__ChannelAssignment> channelAssignment;
    /// <summary>Total number of frames (= samples in each channel) in the file</summary>
    private: std::uint64_t totalFrameCount;

    /// <summary>Target track information container for meta data</summary>
    private: Nuclex::Audio::TrackInfo *trackInfo;
    /// <summary>Current assumed position of the stream decoder's cursor</summary>
    private: std::uint64_t frameCursor;
    /// <summary>Absolute frame index from which the next decode should start</summary>
    private: std::optional<std::uint64_t> scheduledSeekPosition;
    /// <summary>User pointer that will be delivered to the callback</summary>
    private: void *userPointerForCallback;
    /// <summary>Callback that should be invoked to handle the decoded samples</summary>
    private: ProcessDecodedSamplesFunction *processDecodedSamplesCallback;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Flac

#endif // defined(NUCLEX_AUDIO_HAVE_FLAC)

#endif // NUCLEX_AUDIO_STORAGE_FLAC_FLACREADER_H
