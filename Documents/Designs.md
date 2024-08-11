Designs
=======

My goal with Nuclex.Audio.Native is a library that can open audio files
and deliver the data in an a) convenient and understandable way, but also
b) supports streaming and c) allows for and promotes efficient use patterns.

The naive way to expose audio would be to load the whole file into memory.
For special effects audio, that's probably the best, least confusing method.
A 5.1 audio track for a movie can quickly be several Gigabytes, though.
So streaming must be supported, too, any be easy and understandable to use.

Finally, audio is usually stored and/or decoded interleaved, meaning that
samples go round-robin for all channels in a track. For playback and filters,
audio samples should be split into channels. But not for transcoding, however,
transcoding is usually not time-critical (outside of Jellyfin and Plex maybe).

Idea: put interleaved read in `Track` and channel-isolated read in `Channel`?


Random Access Interface
-----------------------

Similar to `VirtualFile` - you just say which samples you want and get them.
If you do it sequentially, performance is great. If you go backwards,
performance becomes abysmal (or not? audio formats work in blocks),
but it still works.

```cpp
class Channel {

  public: template<typename TSample>
  void ReadSamples(TSample *target, std::size_t offset, std::size_t count);

};
```

Thoughts:

  - The total number of samples might not always be known. This could require
    us to revert to the old and cumbersome `ReadUpTo()` design.

  - Multiple channels exist, so a buffering system is needed and knowledge of
    the buffer size (as well as knowledge that there *is* a buffer) to make
    performent use of this.

  - Would work nicely with a `DecodeInFull()` method that decodes everything
    and keeps the data inside the `Channel` classes, closing the audio file.


Stateful Track and Channels
---------------------------

Here, the `Track` would manage a window of decoded samples. So the user of
the library would have to call `DecodeNextSamples(std::size_t sampleCount)`
and then the channels would give access to the next few samples.

```cpp
class Track {

  public: void DecodeNextSamples(std::size_t sampleCount);

  public: std::size_t AvailableSampleCount() const;

  public: void ReadInterleaved(TSample *interleavedBuffer, std::size_t sampleCount);

}
```

Thoughts:

  - To quote Einstein, "spooky action from a distance" - you call some method
    in the `Track` class and all its `Channels` suddenly have different data
    available. Is that confusing?

  - It's good that this gives the user control over buffer sizes. One could
    even expose a method that reads the number of samples that *make sense*
    for the codec (i.e. its natural block size).


Separate Stream Decoder
-----------------------

This could mix with either the random access interface or with keeping any
decoding out of the `Track` and `Channel` classes. Here, a separate decoder
interface would exist that can either be obtained from the `AudioLoader`
separately or perhaps from the `Track` class.

```cpp
class StreamDecoder {

  public: std::size_t DecodeNextSamples(TSample *target, std::size_t bufferSize);
  public: std::uint64_t CountProcessedSamples() const;
  public: std::size_t CountChannels() const;

};
```

Thoughts:

  - The `StreamDecoder` could be implemented on real streaming content, too,
    thus perhaps opening the library for more uses

  - Is this perhaps what a user of the library would expect (principle of
    least surprise on the design level)? Random access to audio samples may
    not be a common request at all but streamed decoding might be expected.


Page In / Page Out
------------------

This is a variation of the "stateful track and channels" idea. Rather than
just have a window that slides forward, segments of the file can be decoded
and become available.

Thoughts:

  - Nope. If the format can be decoded in chunks, the random access design
    completely covers this without forcing the user to manage paging.


Concern: Recompose Channels
---------------------------

Would it be nice if this library would let users just recompose channels,
i.e. add and remove channels from a track in any way the mood strikes them,
then have the library handle the accesses to possibly many input files and
reading them in an efficient way?

Thoughts:

  - This could be pretty nice and lay the groundwork for basic audio
    processing, i.e. a `CompositeChannel` that merged multiple channels with
    adjustable weights, allowing easy stereo downmix etc. with this library.

  - This would mix well with the random access design, which itself would
    work nicely if blocks / chunks make random access feasible and efficient.

  - The biggest difficulty may be to have channels (which can now be
    independent) which share the same source file decode their chunks only
    once. This might have to be carried down all the way into the public API.
