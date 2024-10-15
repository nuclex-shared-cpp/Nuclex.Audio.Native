Nuclex.Audio.Native ![Developed on Linux, should work on Windows](./Documents/images/platforms-linux-windows-badge.svg) ![Design is not final and some parts of the code are still work in progress](./Documents/images/status-design-still-settling-badge.svg)
===================

This library encodes and decodes common audio file formats. It is intended
for desktop applications and video games that wish to process audio in
different codecs. It is not designed for network streaming.

  - ![Tidy](./Documents/images/feature-tidy.svg)
    A lean and well-designed C++ interface that is easy to understand.
  - ![Modular](./Documents/images/feature-modular.svg)
    Modular codec support, you can register custom codec implementations.
  - ![Robust](./Documents/images/feature-robust.svg)
    Robust library design: checks all results and forwards error messages.
  - ![Tested](./Documents/images/feature-tested.svg)
    Unit tested code

File formats

  - Built-in support for Waveform audio files (`.wav`).
  - Default build also includes Ogg Vorbis (`.ogg`), Opus (`.opus`),
    Flac (`.flac`) and WavPack (`.wv`) support.
  - All third-party libraries built from source and embedded for
    minimal runtime dependencies.

Functionality

  - Quickly query metadata from audio files - title, sample format,
    duration, channel count and layout, sample rate and more.
  - Type-safe decoding to `uint8`, `int16`, `int32`, `float` or `double`
    using the most direct path.
  - Sample format conversion via optimized SSE2 SIMD instructions
    and fallback for non-x64 platforms.
  - Detects the file format when opening an audio file.
  - Opens audio files via paths or through a custom I/O interface.
  - Random access in audio data, decodes from any point as needed.

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

// The above soft-fails if the file format is not recognized. If the result
// is there, you get the full set of informations about your audio file.
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

// But the internal conversion is as efficient as it can be,
// so maybe just request them in the format you need them in :)
std::vector<float> samples;
samples.resize(48000 * 2); // two channels, 1 second at 48000 Hz

// Request 48000 interleaved samples as floating point
decoder->DecodeInterleaved<float>(samples.data(), 0, 48000);
// Or request 48000 interleaved samples starting 2 seconds in
decoder->DecodeInterleaved<float>(samples.data(), 96000, 48000);
```


Example: Use Custom I/O Routines
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

This immediately removes all threading concerns from the interface, the same
opened file can be shared by any number of threads. It also allows for
const-correct reading. A `shared_ptr<const VirtualFile>` is fully usable
because pure readers don't need to alter state with a `Seek()` method.

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
