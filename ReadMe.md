Nuclex.Audio.Native ![Developed on Linux, should work on Windows](./Documents/images/platforms-linux-windows-badge.svg) ![Brainstorming, studying the problem space, API slowly taking shape](./Documents/images/status-early-planning-badge.svg)
===================

This library encodes and decodes common audio file formats. It is intended
for desktop applications and video games that wish to process audio in
different codecs. It is not designed for network streaming.

  - A lean and clean C++ interface that is easy to understand.
  - Modular codec support, you can register custom codec implementations.
  - Query information like sample format, channel layout and duration.
  - Built-in support for Waveform audio files (`.wav`).
  - Default build includes Ogg Vorbis, Opus, Flac and WavPack support.
  - By default embeds all third-party libraries for minimal dependencies.
  - Auto-detects file format when opening an audio file.
  - Opens audio files via paths or through a custom I/O interface.
  - Allows random access in audio data, decodes from any point as needed.
  - Delivers audio samples as integers (1-32 bits) or floats (32/64 bits).
  - Robust library design that properly carries exceptions across C code.
  - Unit tested code

It's distinguishing feature is probably how easy it is to use without
sacrificing features or efficiency for it.

**Compiling instructions are found below the examples!**


Missing features
----------------

This library is still a work-in-progress. The Following features are not
completed yet

  - Audio sample conversion.
    *Only floating point samples can be read from decoders currently.*
  - Unit test coverage.
    *It's pretty low at the moment.*
  - Encoding interface.
    *So far, this library can't save or encode at all.*
  - High-level audio channel classes.
    *Only non-functional sketches of these exist for now.*


Example: Query Audio File Properties
------------------------------------

Here is a quick example that shows how you can query various informations
from an audio file, including its duration, channel count, channel layout,
sample rate, data format, fidelity and title:

```cpp
// The AudioLoader handles opening audio files. Audio codecs are registered
// to it (all built-in codecs are registered by default). When opening
// an unknown file, it asks all registered codecs whether they can open it.
AudioLoader loader;

// TryReadInfo() queries information about an audio file in the quickest
// way supported by a codec and without reading the entire audio file.
std::optional<ContainerInfo> info = loader.TryReadInfo("example.flac");

// It soft-fails if the file format is not recognized. If the result is
// non-empty, you get the full set of information about the audio file.
if(info.has_value()) {
  TrackInfo &trackInfo = info.value().Tracks[0];

  cout << "Number of Channels: " << trackInfo.ChannelCount << endl;
  cout << "Duration: " << trackInfo.Duration.count() << " μs" << endl;
  cout << "Channel layout: "
       << StringFromChannelPlacement(trackInfo.ChannelPlacements) << endl;
} else {
  cerr << "Audio file format not supported" << endl:
}
```


Example: Decode Audio by Random Access
--------------------------------------

Getting down to the audio data is quick and easy. You can request the audio
data in the exact format you wish to process it and all conversions will be
taken care of internally, or you may query for how the data is represented
internally and request it in that format to minimize conversions!

```cpp
AudioLoader loader;

// The AudioTrackDecoder is the low-level decoding interface to the library
// and will give you an easy way to directly obtain decoded audio samples.
std::shared_ptr<AudioTrackDecoder> decoder = loader.OpenDecoder("example.opus");

// You could use these informations to request audio samples in their native format
AudioDataFormat nativeFormat = decoder->GetNativeSampleFormat();
bool isInterleaved = decoder->IsNativelyInterleaved();

// But the internal conversion is as efficient as it can be, so just request
// them in the format you need them in:
std::vector<float> samples;
samples.resize(44100 * 2); // two channels, 1 second at 44100 Hz

// Request 48000 interleaved samples as floating point
decoder->DecodeInterleaved<float>(samples.data(), 0, 48000);
// Or request 48000 interleaved samples starting 2 seconds in
decoder->DecodeInterleaved<float>(samples.data(), 96000, 48000);
```


Example: Use custom I/O Routines
--------------------------------

Wherever you have seen a path or file name in the examples before, you can
also specify your own I/O interface to read data from. This is really easy
because you just need to implement an abstract class with three methods:

```cpp
class MyFileWrapper : public VirtualFile {

  public: MyFileWrapper(const std::string &path) :
    fileDescriptor(::open(path.c_str(), O_RDONLY | O_LARGEFILE)) {}
  
  public: ~MyFileWrapper() {
    ::close(this->fileDescriptor);
  }

  // #1 Query current file size
  public: std::uint64_t GetSize() const override {
    struct ::stat fileStatus;
    ::fstat(fileDescriptor, &fileStatus);
    static_cast<std::uint64_t>(fileStatus.st_size);
  }

  // #2 Read data from absolute offset
  public: void ReadAt(
    std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
  ) const override {
    ::pread(this->fileDescriptor, buffer, byteCount, start);      
  }

  // #3 Write data to absolute offset (not needed here)
  public: void WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
  ) override { throw std::runtime_error(u8"Not implemented"); }

};

// Here's a short usage example. Note that we skipped all error handling for
// brevity in the clas above.
int main() {
  AudioLoader loader;

  std::shared_ptr<AudioTrackDecoder> decoder = loader.OpenDecoder(
    std::make_shared<MyFileWrapper>(u8"audio-file.ogg")
  );
}
```

As you can see from the interface, reads and writes are *cursorless*. That
means there is no *current position* and no seeking, the I/O interface is
simply asked to provide data from an absolute offset.

Note how this immediately removes all threading concerns for the interface,
the same opened file can be shared by any number of threads. It also allows
for const-correct reading. A `shared_ptr<const VirtualFile>` is fully usable
because pure readers don't need to alter its state with a `Seek()` method.

The `AudioTrackDecoder` follows the same design principle.

It is fine to use good error handling practices and throw exceptions inside
the `ReadAt()` and `WriteAt()` methods. These will be appropriately carried
over internal C callbacks and propagate out, i.e. from `DecodeInterleaved()`.


Compiling
=========

Nuclex.Audio.Native compiles and embeds all of the third-party audio codecs
from source, so there is a ton of dependencies.

The easiest way to get a directory tree that's ready to compile is to do
a clone (with `--recurse-submodules`!) of my `framework-package` repository:

    git clone --recurse-submodules https://github.com/nuclex-shared-cpp/framework-package.git
    cd framework-package
    cd Nuclex.Audio.Native
    ./build.sh

If you're on Windows and using Visual Studio, instead of running `build.cmd` you can
also just open the `.sln` file and hit *Build All* to compile everything.

    framework-package (msvc-14.2).sln

*Notice: The Windows / Visual Studio projects often lag behind and might miss newly
added source files.*
