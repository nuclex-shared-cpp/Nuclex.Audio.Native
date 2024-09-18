#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_AUDIO_RESOURCEDDIRECTORYLOCATOR_H
#define NUCLEX_AUDIO_RESOURCEDDIRECTORYLOCATOR_H

#include "Nuclex/Audio/Config.h"

#include <string> // for std::string

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Figures out the path to the project's 'Resources' directory</summary>
  /// <returns>The absolute path to the 'Resources' directory</returns>
  /// <remarks>
  ///   <para>
  ///     This library comes with a bunch of test audio files that are used throughout
  ///     the unit tests. Due to their size and platform differences regarding embedded
  ///     resources from binary files, these test files are kept as plain files in
  ///     a subdirectory under the project itself.
  ///   </para>
  ///   <para>
  ///     <code>Nuclex.Audio.Native/Resources/example-audio-file.wav</code>
  ///   </para>
  ///   <para>
  ///     Different unit testing IDE integrations will execute the unit test executable
  ///     either with the project directory as the current directory, or with the directory
  ///     in which the unit tests executable is stored.
  ///   </para>
  ///   <para>
  ///     This method figures out the correct path by taking the directory of the running
  ///     unit test executable, going up 2 levels, then into the 'Resources' directory.
  ///     All based on the assumption that the unit test executable is two directory levels
  ///     below the project's directory (which is always the case if the library is compiled
  ///     with its own build scripts:
  ///   </para>
  ///   <para>
  ///     <code>Nuclex.Audio.Native/bin/linux-amd64-gcc13.3-debug/</code>
  ///   </para>
  /// </remarks>
  std::string GetResourcesDirectory();

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio

#endif // NUCLEX_AUDIO_RESOURCEDDIRECTORYLOCATOR_H
