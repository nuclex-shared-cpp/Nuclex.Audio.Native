Nuclex.Audio.Native ![Developed on Linux, should work on Windows](./Documents/images/platforms-linux-windows-badge.svg) ![Brainstorming, studying the problem space, API slowly taking shape](./Documents/images/status-early-planning-badge.svg)
===================

This is a library in its early stages. I'm exploring possible designs and
studying the topic to find the optimal interface for the library.

When and if completed, it will be something akin to libsndfile with
a C++ API and modular codec support.


Compiling
---------

If you want to compile this, the easiest way is to do a clone
(with `--recurse-submodules`!) of my `framework-package` repository.
It will set up the directory structure and third-party libraries
(built from source) needed to compile this library.

    git clone --recurse-submodules https://github.com/nuclex-shared-cpp/framework-package.git
    cd framework-package
    cd Nuclex.Audio.Native
    ./build.sh

If you're on Windows and using Visual Studio, instead of running `build.cmd` you can
also compile everything by simply opening the `.sln` file and hitting *Build All*.

    framework-package (msvc-14.2).sln

*Notice: The Windows / Visual Studio projects often lag behind and might miss newly
added source files.*


Early Development!
------------------

This is an exploratory project, the design is not final and most functionality
isn't in yet (at the time I'm writing this, you can query things like sample
rate, data format, channel count, channel layout/placements and duration from
FLAC (`.flac`), Opus (`.opus`), Waveform (`.wav`) and WavPack (`.wv`) files
and decode float samples from WavPack, nothing more).


Check the unit tests in the `Tests` directory for some usage samples.

My current concerns are

* querying an audio file for basic information, such as tracks, languages,
  number of channels, sample rate, bit depth and channel placements / layout
  (!) but without decoding the whole audio file

* finding a good solution to expose the audio data

  streaming should be possible, conceivably through a separate interface
  easy data access to possibly deinterleaved channel data would be nice

* once the `AudioCodec` class takes shape, support at least the formats
  I need for my applications: `.wav`, `.flac`, `.wv` (WavPack) and OPUS.

* possibly encoding support, too, using progress notification callbacks
  and the new C++ stop_token to allow for cancelation


Usage Examples
--------------

The `AudioLoader` is the central class you use for decoding. You can register your
own audio codecs (inheriting from the `AudioCodec` class) to it and by default,
a new instance already has all codecs registered that were enabled at compile time.

You can query informations on an audio file like this:

```cpp
AudioLoader loader;

// Will automatically detect the file format. Currently supports
// Flac (.flac), Opus (.opus), Waveform (.wav) and WavPack (.wv)
std::optional<ContainerInfo> info = loader.TryReadInfo("audiofile.wav");

if(info.has_value()) {
  cout << "Supported audio file found" << endl;
  cout << "Number of Channels: " << info.value().Tracks[0].ChannelCount << endl;
  cout << "Channel layout: " <<
    StringFromChannelPlacement(info.value().Tracks[0].ChannelPlacements) << endl;
  cout << "Playback duration: " << info.value().Tracks[0].Duration.count() <<
    "μs" << endl;
}
```

At the time I'm writing this, the library is still a work-in-progress.
For audio decoding, only the low-level `AudioTrackDecoder` interface is available
and only decoding to floating point samples is implemented right now:

```cpp
AudioLoader loader;

// Will automatically detect the file format. Currently supports
// Flac (.flac), Opus (.opus), Waveform (.wav) and WavPack (.wv)
std::shared_ptr<AudioTrackDecoder> decoder = loader.OpenDecoder("stereo.flac");

std::vector<float> interleavedAudioSamples;
interleavedAudioSamples.resize(44100 * 2); // two channels, 1 second at 44100 Hz

decoder->DecodeInterleaved<float>(
  interleavedAudioSamples.data(),
  0, // start frame index (yes, this is random access)
  44100 // number of frames to decode
);

// Now one second of audio samples for two channels, interleaved, have been
// written into the 'interleavedAudioSamples' buffer.
```

Wherever there's a file name, you can also provide a pointer to a `VirtualFile`,
which is a very simple I/O interface with only three methods to implement.
