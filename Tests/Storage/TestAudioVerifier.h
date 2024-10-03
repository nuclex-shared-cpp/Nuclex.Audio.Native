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

#ifndef NUCLEX_AUDIO_STORAGE_TESTAUDIOVERIFIER_H
#define NUCLEX_AUDIO_STORAGE_TESTAUDIOVERIFIER_H

#include "Nuclex/Audio/Config.h"

#include <cstddef> // for std::size_t
#include <vector> // for std::vector

namespace Nuclex { namespace Audio { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class TestAudioVerifier {

    public: static void VerifyStereo(
      const std::vector<float> &samples, std::size_t channelCount,
      std::size_t sampleRate = 48000
    );

    public: static void VerifyStereo(
      const std::vector<float> &leftSamples, const std::vector<float> &rightSamples,
      std::size_t sampleRate = 48000
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Audio::Storage

#endif // NUCLEX_AUDIO_STORAGE_TESTAUDIOVERIFIER_H
