#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.15)

# -------------------------------------------------------------------------------------------------

project(
  NuclexAudioNative
  VERSION 1.0.0
  DESCRIPTION "Extensible library for reading and writing audio files"
)

option(
  BUILD_DOCS
  "Whether to generate documentation via Doxygen"
  OFF
)

option(
  BUILD_UNIT_TESTS
  "Whether to build the unit test executable. This will require an extra \
  compilation of the entire source tree as well as the GoogleTest library."
  OFF
)
option(
  BUILD_BENCHMARK
  "Whether to build the benchmark executable. This will require an extra \
  compilation of the entire source tree as well as the Celero library."
  OFF
)


set(WANT_OPUS ON CACHE BOOL "Whether to integrate Opus audio decoding and encoding")
set(WANT_FLAC OFF CACHE BOOL "Whether to integrate FLAC audio decoding and encoding")
set(WANT_WAVPACK ON CACHE BOOL "Whether to integrate WAVPACK audio decoding and encoding")
set(WANT_VORBIS OFF CACHE BOOL "Whether to integrate Vorbis audio decoding and encoding")

set(WANT_OGG OFF CACHE BOOL "Whether to support the OGG container format")
set(WANT_M4A OFF CACHE BOOL "Whether to support the M4A (MP4) container format")
set(WANT_MATROSKA OFF CACHE BOOL "Whether to support the Matroska container format")

# -------------------------------------------------------------------------------------------------

# This sets a bunch of compile flags and defined ${NUCLEX_COMPILER_TAG} to
# say something like linux-gcc-13.2-amd64-debug. You should have this directory
# if you do a full clone of a project that is using this third-party library build.
include("../build-system/cmake/cplusplus.cmake")

# The Unix build pipeline doesn't automatically include threading, so search for
# the pthreads library in order to link against it later on.
#   https://en.wikipedia.org/wiki/Pthreads
find_package(Threads REQUIRED)

# Add Nuclex.Support.Native as a sub-project, we link it for utility methods.
if(NOT (TARGET NuclexSupportNative))
  add_subdirectory(
    ${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native
    ${CMAKE_BINARY_DIR}/NuclexSupportNative
  )
endif()

message(STATUS "Enabled options for Nuclex.Audio.Native:")
message(STATUS "  ⚫ Build core library")

if(BUILD_UNIT_TESTS)
  message(STATUS "  ⚫ Build unit tests")

  # Add GoogleTest as a sub-project so we can link our unit test executable
  if(NOT (TARGET GoogleTest))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-googletest
      ${CMAKE_BINARY_DIR}/nuclex-googletest
    )
  endif()
endif()

if(BUILD_BENCHMARK)
  message(STATUS "  ⚫ Build benchmark")

  # Add Celero as a sub-project so we can link our benchmark executable
  if(NOT (TARGET Celero))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-celero
      ${CMAKE_BINARY_DIR}/nuclex-celero
    )
  endif()
endif()

# Libraries to load and save different audio formats
if(WANT_OPUS)
	message(STATUS "  ⚫ Support Opus")
  if(NOT (TARGET Ogg))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-ogg
      ${CMAKE_BINARY_DIR}/nuclex-ogg
    )
  endif()
  if(NOT (TARGET Opus))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-opus
      ${CMAKE_BINARY_DIR}/nuclex-opus
    )
  endif()
  if(NOT (TARGET OpusFile))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-opusfile
      ${CMAKE_BINARY_DIR}/nuclex-opusfile
    )
  endif()
endif()
if(WANT_FLAC)
	message(STATUS "  ⚫ Support FLAC")
  if(NOT (TARGET Flac))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-flac
      ${CMAKE_BINARY_DIR}/nuclex-flac
    )
  endif()
endif()
if(WANT_WAVPACK)
	message(STATUS "  ⚫ Support WavPack")
  if(NOT (TARGET WavPack))
    add_subdirectory(
      ${PROJECT_SOURCE_DIR}/../third-party/nuclex-wavpack
      ${CMAKE_BINARY_DIR}/nuclex-wavpack
    )
  endif()
endif()

# Use CMake's own package for locating Doxygen on the system
if(BUILD_DOCS)
  find_package(Doxygen)
endif()

# -------------------------------------------------------------------------------------------------

# Project structure
#
#   ProjectName/
#     Source/                   All source files, using deeper directories as needed
#     Include/ProjectName/      All public headers, using deeper directories as needed
#     Tests/                    All unit tests, using deeper directories as needed
#     Benchmarks/               All benchmark files, using deeper directories as needed
#
# CMake documentation:
#   |  Note: We do not recommend using GLOB to collect a list of
#   |  source files from your source tree. If no CMakeLists.txt file
#   |  changes when a source is added or removed then the generated
#   |  build system cannot know when to ask CMake to regenerate.
#
# As so very often, CMake becomes a hurdle rather than helping.
# I'm not going to manually maintain a list of source files. Rebuilds
# where files are added, removed or renamed need to be from scratch.
#
file(
  GLOB_RECURSE sourceFiles
  CONFIGURE_DEPENDS
  "Source/*.cpp"
  "Source/*.c"
)
file(
  GLOB_RECURSE headerFiles
  CONFIGURE_DEPENDS
  "Include/Nuclex/Audio/*.h"
)
file(
  GLOB_RECURSE unittestFiles
  CONFIGURE_DEPENDS
  "Tests/*.cpp"
)
file(
  GLOB_RECURSE benchmarkFiles
  CONFIGURE_DEPENDS
  "Benchmarks/*.cpp"
)

# -------------------------------------------------------------------------------------------------

function(add_third_party_libraries target_name)

  # Files need to be linked in reverse order of dependency.
  #
  # Example: LibOpusFile depends on LibOgg.
  #
  # If you link LibOgg first, the linker will remove most of LibOgg as unused symbols,
  # even those sybmols that will thereafter be missing when LibOpusFile gets linked.
  #
  # Thus, first link LibOpusFile, letting the linker take note of the dangling references
  # to various LibOgg symbols, then link LibOgg, letting it connect those references.
  #
  if(WANT_OPUS)
    target_compile_definitions(${target_name} PRIVATE NUCLEX_AUDIO_HAVE_OPUS)
    target_link_libraries(${target_name} PRIVATE OpusFile::Static)
    target_link_libraries(${target_name} PRIVATE Opus::Static)
    target_link_libraries(${target_name} PRIVATE Ogg::Static)
  endif()
  if(WANT_FLAC)
    target_compile_definitions(${target_name} PRIVATE NUCLEX_AUDIO_HAVE_FLAC)
    target_link_libraries(${target_name} PRIVATE Flac::Static)
  endif()
  if(WANT_WAVPACK)
    target_compile_definitions(${target_name} PRIVATE NUCLEX_AUDIO_HAVE_WAVPACK)
    target_link_libraries(${target_name} PRIVATE WavPack::Static)
  endif()

  # On Unix systems, the library and unit test executable should look for
  # dependencies in its own directory first.
  set_target_properties(
    ${target_name} PROPERTIES
    BUILD_RPATH_USE_ORIGIN ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH OFF
    INSTALL_RPATH "\${ORIGIN}"
  )

endfunction()

# -------------------------------------------------------------------------------------------------

# Shared library that can be linked to other projects
add_library(NuclexAudioNative SHARED)

# Enable compiler warnings only if this library is compiled on its own.
# If it's used as a sub-project, the including project's developers aren't
# interested in seeing warnings from a project they're not maintaining.
if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexAudioNative")
  enable_target_compiler_warnings(NuclexAudioNative)
else()
  disable_target_compiler_warnings(NuclexAudioNative)
endif()

# Add directory with public headers to include path
target_include_directories(
  NuclexAudioNative
  PUBLIC "Include"
)

# Add public headers and sources to compilation list
# (headers, too, in case CMake is used to generate an IDE project)
target_sources(
  NuclexAudioNative
  PUBLIC ${headerFiles}
  PRIVATE ${sourceFiles}
)

# Link against PThreads and Nuclex.Support.Native
target_link_libraries(
  NuclexAudioNative
  PRIVATE Threads::Threads
  PUBLIC NuclexSupportNative
)

# Add include directories and static libraries of all enabled image formats
add_third_party_libraries(NuclexAudioNative)

# On Windows, we want the shared library to be named Nuclex.Audio.Native.dll
if(WIN32)
  set_target_properties(
    NuclexAudioNative
    PROPERTIES OUTPUT_NAME "Nuclex.Audio.Native"
  )
endif()

# -------------------------------------------------------------------------------------------------

if(BUILD_UNIT_TESTS)

  # Executable that runs the unit tests (main() supplied by GoogleTest)
  add_executable(NuclexAudioNativeTests)

  # Enable compiler warnings only if this library is compiles on its own.
  # If it's used as a sub-project, the including project's developers aren't
  # interested in seeing warnings from a project they're not maintaining.
  if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexAudioNative")
    enable_target_compiler_warnings(NuclexAudioNativeTests)
  else()
    disable_target_compiler_warnings(NuclexAudioNativeTests)
  endif()

  # Let the code know it's not being compiled into a shared library
  # (this disables visibility/exports, thus allowing the compiler detect
  # additional unused code and warn about it)
  target_compile_definitions(
    NuclexAudioNativeTests
    PRIVATE NUCLEX_AUDIO_EXECUTABLE
  )

  # Add directory with public headers to include path
  target_include_directories(
    NuclexAudioNativeTests
    PUBLIC "Include"
  )

  # Add public headers and sources (normal + unit tests) to compilation list
  # (headers, too, in case CMake is used to generate an IDE project)
  target_sources(
    NuclexAudioNativeTests
    PRIVATE ${headerFiles}
    PRIVATE ${sourceFiles}
    PRIVATE ${unittestFiles}
  )

  # Link GoogleTest and the main() function supplied by GoogleTest
  # Also link against PThreads
  target_link_libraries(
    NuclexAudioNativeTests
    PRIVATE GoogleTest::Static
    PRIVATE GoogleTest::Main
    PRIVATE Threads::Threads
    PUBLIC NuclexSupportNative
  )

  add_third_party_libraries(NuclexAudioNativeTests)

  # On Windows, we want the executable to be named Nuclex.Audio.Native.Tests.exe
  if(WIN32)
    set_target_properties(
      NuclexAudioNativeTests
      PROPERTIES OUTPUT_NAME "Nuclex.Audio.Native.Tests"
    )
  endif()

endif() # if BUILD_UNIT_TESTS enabled

# -------------------------------------------------------------------------------------------------

if(BUILD_BENCHMARK)

  # Executable that runs the benchmark (main() supplied by Celero)
  add_executable(NuclexAudioNativeBenchmark)

  # Enable compiler warnings only if this library is compiled on its own.
  # If it's used as a sub-project, the including project's developers aren't
  # interested in seeing warnings from a project they're not maintaining.
  if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexAudioNative")
    enable_target_compiler_warnings(NuclexAudioNativeBenchmark)
  else()
    disable_target_compiler_warnings(NuclexAudioNativeBenchmark)
  endif()

  # Let the code know it's not being compiled into a shared library
  # (this disables visibility/exports, thus allowing the compiler detect
  # additional unused code and warn about it)
  target_compile_definitions(
    NuclexAudioNativeBenchmark
    PRIVATE NUCLEX_AUDIO_EXECUTABLE
  )

  # Add directory with public headers to include path
  target_include_directories(
    NuclexAudioNativeBenchmark
    PUBLIC "Include"
  )

  # Add public headers and sources (normal + benchmark) to compilation list
  # (headers, too, in case CMake is used to generate an IDE project)
  target_sources(
    NuclexAudioNativeBenchmark
    PRIVATE ${headerFiles}
    PRIVATE ${sourceFiles}
    PRIVATE ${benchmarkFiles}
  )

  # Link Celero
  # Also link against PThreads
  target_link_libraries(
    NuclexAudioNativeBenchmark
    PRIVATE Celero
    PRIVATE Threads::Threads
    PUBLIC NuclexSupportNative
  )

  add_third_party_libraries(NuclexAudioNativeBenchmark)

  # On Windows, we want the executable to be named Nuclex.Audio.Native.Benchmark.exe
  if(WIN32)
    set_target_properties(
      NuclexAudioNativeBenchmark
      PROPERTIES OUTPUT_NAME "Nuclex.Audio.Native.Benchmark"
    )
  endif()

endif() # if BUILD_BENCHMARK enabled

# -------------------------------------------------------------------------------------------------

# Install the shared library into a subdirectory of this CMakeLists.txt file
# under ./bin/linux-gcc9.3-amd64-debug/ (the second-level directory is called
# "compiler tag" and dynamically formed -- it ensures that when linking
# a pre-compiled shared library, the correct library is used).
install(
  TARGETS NuclexAudioNative
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Install .pdb files on Windows audios for the main library.
install_debug_symbols(NuclexAudioNative)

# Do the same for Nuclex.Support.Native. Since we depend on this library
# and have set the rpath accordingly, it needs to be in the same directory
install(
  TARGETS NuclexSupportNative
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Install unit tests in same location as shared library.
if(BUILD_UNIT_TESTS)
  install(
    TARGETS NuclexAudioNativeTests
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  )

  # Install .pdb files on Windows audios for the unit tests, too.
  install_debug_symbols(NuclexAudioNativeTests)
endif()

# Install benchmarks in same location as shared library.
if(BUILD_BENCHMARK)
  install(
    TARGETS NuclexAudioNativeBenchmark
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
  )

  # Install .pdb files on Windows audios for the benchmark, too.
  install_debug_symbols(NuclexAudioNativeBenchmark)
endif()

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
  endif()

  add_custom_target(
    NuclexAudioNativeDocs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Audio.Native.doxygen.cfg"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )

endif()

# -------------------------------------------------------------------------------------------------

file(
  WRITE "${PROJECT_SOURCE_DIR}/NuclexAudioNativeConfig.cmake"
  "#!/usr/bin/cmake

# Configuration to include Nuclex.Audio.Native in a CMake-based project. If you want to
# reference Nuclex.Audio.Native as an externally compiled static library, do this:
#
#    set(NuclexAudioNative_DIR \"../Nuclex.Audio.Native\")
#    find_package(NuclexAudioNative REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexAudioNative::Dynamic
#    )
#
# Alternatively, if you want to build Nuclex.Audio.Native together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../Nuclex.Audio.Native\"
#        \"\${CMAKE_BINARY_DIR}/nuclex.audio.native\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexAudioNative
#    )
#

# -------------------------------------------------------------------------------------------------

if(NOT DEFINED NUCLEX_COMPILER_TAG)
	message(
		FATAL_ERROR
		\"NUCLEX_COMPILER_TAG not defined! Include cplusplus.cmake before importing this package \\
		in order to generate a tag identifying the audio/compiler/architecture/variant!\"
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}\")
	# TODO: Warn and link release build when compiling in debug mode
	# TODO: Warn and link build for older compiler version if found
	message(
		FATAL_ERROR
		\"Directory '\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}' not found. \\
		Please either build and install this project before importing it via \\
		find_package() or use this project's main CMakeFiles.txt via add_subdirectory()!\"
	)
endif()

# -------------------------------------------------------------------------------------------------

add_library(NuclexAudioNative::Dynamic SHARED IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	NuclexAudioNative::Dynamic PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		NuclexAudioNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/Nuclex.Audio.Native.lib\"
	)
else()
	set_target_properties(
		NuclexAudioNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libNuclexAudioNative.so\"
	)
endif()

message(STATUS \"Imported Nuclex.Audio.Native targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
