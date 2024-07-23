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
            nuclex-googletest/      <-- Git: nuclex-builds/nuclex-googletest
                CMakeLists.txt
            nuclex-celero/          <-- Git: nuclex-builds/nuclex-ceelero
                CMakeLists.txt

If that's a bit overwhelming, try cloning (with `--recurse-submodules`) my "opus-transcoder"
or another application that uses this library to get that directory tree.

The dependencies of the code itself are rather tame:

  * Nuclex.Support.Native
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
