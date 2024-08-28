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

*The Windows / Visual Studio projects usually lag behind and miss newly added
source files. They also require you to build all third-party libraries with
CMake by hand if you intend to compile inside Visual Studio itself.*


Early Development!
------------------

This is an exploratory project, the design is not final and most functionality
isn't in yet (at the time I'm writing this, you can query things like sample
rate, data format, channel count, channel layout/placements and duration from
Waveform (`.wav`), WavPack (`.wv`) and Opus (`.opus`) files, nothing more).

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

