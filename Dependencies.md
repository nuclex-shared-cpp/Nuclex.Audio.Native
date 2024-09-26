Nuclex.Audio.Native Dependencies
================================


To Compile the Library
----------------------

This library is intended to be placed into a source tree using submodules to replicate
the following directory layout:

    root/
        Nuclex.Audio.Native/        <-- you are here
            CMakeLists.txt

        Nuclex.Support.Native/      <-- Git: nuclex-shared-cpp/Nuclex.Support.Native
            CMakeLists.txt

        build-system/               <-- Git: nuclex-shared/build-system
            cmake/
                cplusplus.cmake

        third-party/
            nuclex-flac/            <-- Git: nuclex-builds/nuclex-flac
                CMakeLists.txt
            nuclex-ogg/             <-- Git: nuclex-builds/nuclex-ogg
                CMakeLists.txt
            nuclex-opus/            <-- Git: nuclex-builds/nuclex-opus
                CMakeLists.txt
            nuclex-opusfile/        <-- Git: nuclex-builds/nuclex-opusfile
                CMakeLists.txt
            nuclex-opusenc/         <-- Git: nuclex-builds/nuclex-opusenc
                CMakeLists.txt
            nuclex-vorbis/          <-- Git: nuclex-builds/nuclex-vorbis
                CMakeLists.txt
            nuclex-wavpack/         <-- Git: nuclex-builds/nuclex-wavpack
                CMakeLists.txt
            nuclex-googletest/      <-- Git: nuclex-builds/nuclex-googletest
                CMakeLists.txt
            nuclex-celero/          <-- Git: nuclex-builds/nuclex-celero
                CMakeLists.txt

If that's a bit overwhelming, try cloning (with `--recurse-submodules`) my "opus-transcoder"
or another application that uses this library to get that directory tree.

The dependencies of the code itself involve all of the popular audio libraris:

  * Nuclex.Support.Native
  * libogg (optional, if the vorbis or opus codecs are enabled)
  * libvorbis (optional, if the vorbis codec is enabled)
  * libvorbisfile (optional, if the vorbis codec is enabled)
  * libvorbisenc (optional, if the vorbis codec is enabled)
  * libopus (optional, if the opus codec is enabled)
  * libopusfile (optional, if the opus codec is enabled)
  * libopusenc (optional, if the opus codec is enabled)
  * libflac (optional, if the flac codec is enabled)
  * libwavpack (optional, if the wavpack codec is enabled)
  * gtest (optional, if unit tests are built)
  * celero (optional, if benchmarks are built)

To Use this Library as a Binary
-------------------------------

Either use the included `CMakeLists.txt` (it still requires the `build-system` directory)
or, more directly:

  * Add `Nuclex.Audio.Native/Include` to your include directory
  * Add `Nuclex.Support.Native/Include` to your include directory
  * Link `libNuclexAudioNative.so` (or `Nuclex.Audio.Native.lib` on Windows)
  * Copy the `.so` files (or `.dll` fils on Windows) to your output directory
    (of both `Nuclex.Audio.Native` and `Nuclex.Support.Native` which it depends on)
