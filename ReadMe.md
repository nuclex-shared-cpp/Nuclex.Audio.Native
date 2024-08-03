Nuclex.Audio.Native ![Developed on Linux, should work on Windows](https://img.shields.io/badge/platforms-linux_windows-darkgreen) ![Brainstorming, studying the problem space, API slowly taking shape](https://img.shields.io/badge/status-early_planning_phase-darkmagenta)
===================

This is a library in its early stages. I'm exploring possible designs and
studying the topic to find the optimal design.

When and if completed, it will be something akin to libsndfile with
a C++ API and modular codec support.

My current concerns are

* querying an audio file for basic information, such as tracks, languages,
  number of channels, sample rate, bit depth and channel placements / layout
  without decoding the whole thing

* finding a good solution to expose both the audio data
  streaming should be possible, conceivable through a separate interface
  easy data access to possibly deinterleaved channel data would be nice

* once the codec design takes shape, supporting at least the formats
  I need for my applications: .wav, .flac, .wv (WavPack) and OPUS.

* possibly encoding support, too, using progress notification callbacks
  and the new C++ stop_token to allow for cancelation
