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

#ifndef NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERBUILDER_H
#define NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERBUILDER_H

#include "Nuclex/Audio/Config.h"
#include "Nuclex/Audio/ChannelPlacement.h"
#include "Nuclex/Audio/AudioSampleFormat.h"

#include <vector> // for std::vector
#include <memory> // for std::shared_ptr

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class VirtualFile;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  // This thing should be returned by some 'AudioSaver' (I hate that name, is there
  // a better one that doesn't have 'Encoder' or 'Writer' in it?) class.
  //
  // Said class should allow codecs to be listed and then hand out an encoder builder,
  // the class below, which allows settings for the encoder to be made and then will
  // build the encoder which only needs to be fed with input samples and writes to
  // a VirtualFile.
  //
  // Furthermore, the 'AudioSaver' class should offer the option to directly save
  // a 'Track' (the high level class managing an autonomous or on-the-fly decoding track)
  // and will encode the track with its known settings directly (by filling out
  // the 'AudioTrackEncoderBuilder' itself so the burden is no pushed to individual
  // codec implementations).
  //

  /// <summary>Generates audio track encoders</summary>
  class NUCLEX_AUDIO_TYPE AudioTrackEncoderBuilder {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_AUDIO_API virtual ~AudioTrackEncoderBuilder() = default;

    /// <summary>Retrieves a list of supported formats for the encoded samples</summary>
    /// <returns>All supported formats in which samples can be stored</returns>
    /// <remarks>
    ///   This does not limit the types of samples you can feed into the encoder,
    ///   once built. It contains the formats the encoded file will ultimately store.
    ///   Lossy codecs generally don't have a sample format and they consume and produce
    ///   floating point values. In this case, only float will be listed. Lossy codecs
    ///   will list the sample formats they actually store in the file.
    /// </remarks>
    public: virtual const std::vector<AudioSampleFormat> &GetSupportedSampleFormats() const = 0;

    /// <summary>Retrieves a list of supported sample rates for the codec</summary>
    /// <returns>All supported sample rates or empty if unrestricted</returns>
    /// <remarks>
    ///   <para>
    ///     Lossy codecs are sometimes limited to specific sample rates. That may be
    ///     for reasons of tuning (they may ship with built-in look-up tables listing
    ///     bands to cut off or amplify).
    ///   </para>
    ///   <para>
    ///     If the returned list is empty, the codec will accept any sample rate.
    ///     Otherwise, you have to pick one of the sample rates found in the list.
    ///   </para>
    /// </remarks>
    public: virtual const std::vector<std::size_t> &GetSupportedSampleRates() const = 0;

    /// <summary>Retrieves a list of preferred sample rates for the codec</summary>
    /// <returns>The preferred sample rates or empty if the codec doesn't care</returns>
    /// <remarks>
    ///   <para>
    ///     While some lossy codecs have restricted themselves to specific sample rates,
    ///     others accept anything and simply sound less good, or they sneakily resample
    ///     your audio data to a fixed sample rate, as is the case with Opus.
    ///   </para>
    ///   <para>
    ///     If optimum quality is your goal, do consider resampling the audio signal with
    ///     a high-end resampler (the likely best algorithm conceived can be found in
    ///     the &quot;rubberband&quot; library, by the way).
    ///   </para>
    ///   <para>
    ///     If this list is empty, the codec is neutral. Otherwise, it contains
    ///     the sample rates the codec is intended for, but it will accept other bitrates
    ///     as well  (they just may produce poorer quality and/or cause resampling).
    ///   </para>
    /// </remarks>
    public: virtual const std::vector<std::size_t> &GetPreferredSampleRates() const = 0;

    /// <summary>Retrieves the channel order preferred by the encoder</summary>
    /// <param name="channels">Channels that should be put in the preferred order</param>
    /// <returns>A list of the channel in the mask in their preferred order</returns>
    /// <remarks>
    ///   <para>
    ///     Some audio formats, Vorbis and Opus for example, have their own standard for
    ///     the order in which channels should be interleaved. Feeding the encoder channels
    ///     in this order will avoid having to re-weave the input channels
    ///   </para>
    ///   <para>
    ///     Apart from the performance aspect, you can configure an encoder to any
    ///     channel order you like. If a sample format conversion is needed anyway,
    ///     the performance loss of also re-weaving the channels is neglegible.
    ///   </para>
    /// </remarks>
    public: virtual std::vector<ChannelPlacement> GetPreferredChannelOrder(
      ChannelPlacement channels
    ) const = 0;

    /// <summary>Tells whether this audio codec is a lossless one</summary>
    /// <returns>True if the codec is lossless, false if it is lossy</returns>
    /// <remarks>
    ///   This one probably doens't need much explanation. Lossless audio codecs exactly
    ///   decode the original audio signal bit-perfect but their space savings are so-so
    ///   (files sizes of half or two thirds of the uncompressed audio data). Lossy codecs
    ///   deliver phenomenal space savings, but they don't exactly preserve the audio
    ///   signal, leading to generation loss upon transcoding.
    /// </remarks>
    public: virtual bool IsLossless() const = 0;

    /// <summary>Selects the format in which samples will be stored in the file</summary>
    /// <param name="format">Format to use for the encoded samples</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   <para>
    ///     Some audio codecs, usually of the lossless variety, support storing samples in
    ///     different formats. Lossy codecs normally don't deal with different sample formats
    ///     an effectively only consume and decode floating point data, using whatever
    ///     advanced techniques they do to shrink down the number of bytes in the file.
    ///   </para>
    ///   <para>
    ///     Specifying an unsupported format is a hard error, so either consult the
    ///     <see cref="GetSupportedSampleFormats" /> method or know the capabilities of
    ///     the codec you've picked up front.
    ///   </para>
    ///   <para>
    ///     If this method is not called, the sample format will either be 16-bit integers
    ///     for lossless codecs or floating point for lossy codecs. This has no direct
    ///     correlation to the input format in which you feed the constructed encoder samples,
    ///     though for lossless encoding it is a good idea to have the two match.
    ///   </para>
    /// </remarks>
    public: virtual AudioTrackEncoderBuilder &SetSampleFormat(
      AudioSampleFormat format = AudioSampleFormat::SignedInteger_16
    );

    /// <summary>Tells the encoder the sample rate of your audio data</summary>
    /// <param name="samplesPerSecond">
    ///   Sample rate in samples per second (in each channel)
    /// </param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   <para>
    ///     This call is mandatory because mismatching the sample rate will make your
    ///     audio track sound wrong and because there is no universal standard. Most audio
    ///     for movies has settled on 48000 samples per second, but CD audio is fixed to
    ///     44100 samples per second..
    ///   </para>
    ///   <para>
    ///     While it's no big deal for lossless codecs (which simply have the samples
    ///     as they are and the sample rate is merely an unrelated piece of information),
    ///     the sample rate is vital for lossy codecs which usually prioritize frequencies
    ///     to which the human ear is sensitive. Most either have a limited set of sample
    ///     rates they support or even resample to a unified rate (i.e. Opus always uses
    ///     48000 samples per second for full bandwidth audio).
    ///   </para>
    ///   <para>
    ///     Query <see cref="GetSupportedSampleRates" /> and
    ///     <see cref="GetPreferredSampleRates" /> to figure out which sample rates you
    ///     are allowed to use and which ones result in optimal quality. If you do not
    ///     specify this, the encoder will fail to build with an exception.
    ///   </para>
    /// </remarks>
    public: virtual AudioTrackEncoderBuilder &SetSampleRate(
      std::size_t samplesPerSecond = 48000
    );

    /// <summary>Sets the number, placement and ordering of the input channels</summary>
    /// <param name="orderedChannels">
    ///   A list containing the channels to encode in the order you wish to feed them
    ///   to the encoder.
    /// </param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   <para>
    ///     Most audio codecs (including Waveform, Flac, WavPack, Vorbis and Opus) have
    ///     their own mandatory channel order schemes. This order is for your convenience
    ///     but will very likely have no effect on the order in which the channels will be
    ///     interleaved in the audio file being written.
    ///   </para>
    ///   <para>
    ///     If you specify a channel order that is different form the one the audio codec
    ///     uses, this library will internally create a reordered copy of each chunk before
    ///     feeding it to the encoder. If you wish to avoid the overhead this involves, you
    ///     can use the <see cref="GetPreferredChannelOrder" /> method to obtain the order
    ///     in which the encoder natively accepts the channels.
    ///   </para>
    /// </remarks>
    public: virtual AudioTrackEncoderBuilder &SetChannels(
      const std::vector<ChannelPlacement> &orderedChannels
    );

    /// <summary>Configures the encoder to use standard stereo channels</summary>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   With this channel layout, there will be two channels in total, interleaved
    ///   in the following channel order:
    ///   <list type="number">
    ///     <item><description>Front Left</description></item>
    ///     <item><description>Front Right</description></item>
    ///   </list>
    /// </remarks>
    public: AudioTrackEncoderBuilder &SetStereoChannels();

    /// <summary>Configures the encoder to use standard stereo channels</summary>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   With this channel layout, there will be six channels in total, interleaved
    ///   in the following channel order:
    ///   <list type="number">
    ///     <item><description>Front Left</description></item>
    ///     <item><description>Front Right</description></item>
    ///     <item><description>Front Center</description></item>
    ///     <item><description>Low Frequency Effects (LFE)</description></item>
    ///     <item><description>Back Left (or Side Left for 5.1 side)</description></item>
    ///     <item><description>Back Right (or Side Right for 5.1 side)</description></item>
    ///   </list>
    /// </remarks>
    public: AudioTrackEncoderBuilder &SetFiveDotOneChannels();

    /// <summary>Configures the encoder to use standard stereo channels</summary>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   With this channel layout, there will be six channels in total, interleaved
    ///   in the following channel order:
    ///   <list type="number">
    ///     <item><description>Front Left</description></item>
    ///     <item><description>Front Center</description></item>
    ///     <item><description>Front Right</description></item>
    ///     <item><description>Back Left</description></item>
    ///     <item><description>Back Right</description></item>
    ///     <item><description>Low Frequency Effects (LFE)</description></item>
    ///   </list>
    /// </remarks>
    public: AudioTrackEncoderBuilder &SetFiveDotOneChannelsInVorbisOrder();

    /// <summary>Selects the bitrate which the encoder should try to match</summary>
    /// <param name="kilobitsPerSecond">Desired bit rate in kilobits per second</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   <para>
    ///     Lossy audio codecs mostly use VBR to control their quality (though some encoders
    ///     do allow a target quality to be specified instead). The bitrate may not be hit
    ///     exactly and it is up to the encoder whether it interprets it as a whole-file
    ///     average or a per-N-seconds average.
    ///   </para>
    ///   <para>
    ///     Lossless codecs will completely disregard the requested bit rate.
    ///   </para>
    ///   <para>
    ///     Not specifying the bit rate results in a reasonable bit rate that is considered
    ///     to be transparent (indistinguishable from the original to listeners) being picked.
    ///   </para>
    /// </remarks>
    public: AudioTrackEncoderBuilder &SetTargetBitrate(std::size_t kilobitsPerSecond);

    /// <summary>Requests the amount of effort that should be used to compress</summary>
    /// <param name="effort">Effort as a value from 0.0 to 1.0</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    /// <remarks>
    ///   <para>
    ///     This simply tells the encoder how hard it should crunch numbers to and produce
    ///     the best / most accurate sound per kilobyte of audio data. 
    ///   </para>
    ///   <para>
    ///     For lossy audio codecs, this will improve audio quality for any given bitrate
    ///     at the cost of taking longer to encode. For lossless audio codecs, which always
    ///     output 100% quality and disregard any target bitrates, the file size will shrink
    ///     more with higher values, also at the cost of taking longer to encode.
    ///   </para>
    ///   <para>
    ///     Defaults to maximum because it's 2024 and outside of VoIP or streaming (both
    ///     of which this library is not designed for), any desktop easily handles maximum
    ///     effort at very decent encoding speeds. What are you doing even looking at this?
    ///   </para>
    /// </remarks>
    public: AudioTrackEncoderBuilder &SetCompressionEffort(
      float effort = 1.0f
    );

    /// <summary>Sets the title of the audio track</summary>
    /// <param name="title">Human-readable title of the audio track</param>
    /// <returns>The encoder builder such that additional calls can be chained</returns>
    public: AudioTrackEncoderBuilder &SetTitle(const std::string &title);

    // Do we need the duration early?
    // Writing it in when the encoder stops isn't a big issue,
    // this library doesn't do streaming.


    //public: std::shared_ptr<AudioTrackEncoder> Build();

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_AUDIOTRACKENCODERBUILDER_H
