#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_AUDIO_SOURCE 1

#include "../../../Source/Storage/Opus/OpusTrackDecoder.h"

#if defined(NUCLEX_AUDIO_HAVE_OPUS)

#include "Nuclex/Audio/Storage/VirtualFile.h"
#include "../../../Source/Storage/Opus/OpusTrackEncoderBuilder.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  class DummyVirtualFile : public Nuclex::Audio::Storage::VirtualFile {

    /// <summary>Frees all memory used by the instance</summary>
    public: ~DummyVirtualFile() override = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: virtual std::uint64_t GetSize() const {
      throw std::runtime_error(u8"Querying size not implemented");
    }

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: virtual void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::byte *buffer
    ) const { throw std::runtime_error(u8"Reading not implemented"); }

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: virtual void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::byte *buffer
    ) {
      (void)start;
      (void)byteCount;
      (void)buffer;
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio { namespace Storage { namespace Opus {

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackEncoderBuilderTest, BuildThrowsExceptionWithoutInputChannels) {
    std::shared_ptr<OpusTrackEncoderBuilder> builder = (
      std::make_shared<OpusTrackEncoderBuilder>()
    );

    EXPECT_THROW(
      builder->Build(std::shared_ptr<VirtualFile>()),
      std::runtime_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OpusTrackEncoderBuilderTest, CanBuildEncoder) {
    std::shared_ptr<OpusTrackEncoderBuilder> builder = (
      std::make_shared<OpusTrackEncoderBuilder>()
    );
    
    std::shared_ptr<AudioTrackEncoder> encoder = builder->
      SetStereoChannels().
      SetSampleRate(48000).
      SetTargetBitrate(192.0f).
      Build(std::make_shared<DummyVirtualFile>());

    EXPECT_TRUE(static_cast<bool>(encoder));
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Audio::Storage::Opus

#endif // defined(NUCLEX_AUDIO_HAVE_OPUS)
