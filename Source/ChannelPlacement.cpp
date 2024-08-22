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

#include "Nuclex/Audio/ChannelPlacement.h"

#include <Nuclex/Support/BitTricks.h> // for BitTricks
#include <Nuclex/Support/Text/ParserHelper.h> // for ParserHelper
#include <Nuclex/Support/Text/StringMatcher.h> // for StringMatcher
#include <Nuclex/Support/Text/UnicodeHelper.h> // for UnicodeHElper
#include <Nuclex/Support/Errors/CorruptStringError.h> // for CorruptStringError

#include <string_view> // for std::string_view
#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Tag that indicates a channel has no known placement</summary>
  const std::string_view NoneTag(u8"none", 4);
  /// <summary>Tag that indicates a channel is on the left side</summary>
  const std::string_view LeftTag(u8"left", 4);
  /// <summary>Tag that indicates a channel is on the right side</summary>
  const std::string_view RightTag(u8"right", 5);
  /// <summary>Tag that indicates a channel is in the center</summary>
  const std::string_view CenterTag(u8"center", 6);
  /// <summary>Tag that indicates a channel is to the front</summary>
  const std::string_view FrontTag(u8"front", 5);
  /// <summary>Tag that indicates a channel is to the back</summary>
  const std::string_view BackTag(u8"back", 4);
  /// <summary>Tag that indicates a channel is to the back</summary>
  const std::string_view RearTag(u8"rear", 4);
  /// <summary>Tag that indicates a channel is above</summary>
  const std::string_view TopTag(u8"top", 3);
  /// <summary>Tag that indicates a channel is below</summary>
  const std::string_view BottomTag(u8"bottom", 6);
  /// <summary>Tag that indicates a channel is a subwoofer channel</summary>
  const std::string_view BassTag(u8"bass", 4);
  /// <summary>Tag that indicates a channel is a subwoofer channel</summary>
  const std::string_view LfeTag(u8"lfe", 3);
  /// <summary>Tag that for the word low</summary>
  const std::string_view LowTag(u8"low", 3);
  /// <summary>Tag that for the word frequency</summary>
  const std::string_view FrequencyTag(u8"frequency", 9);
  /// <summary>Tag that for the word effects</summary>
  const std::string_view EffectsTag(u8"effects", 7);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Names of the possible channel placements, ordered by bit index</summary>
  const std::string channelNames[] = {
    u8"front left",
    u8"front right",
    u8"front center",
    u8"low frequency effects",
    u8"back left",
    u8"back right",
    u8"front center left",
    u8"front center right",
    u8"back center",
    u8"side left",
    u8"side right",
    u8"top center",
    u8"top front left",
    u8"top front center",
    u8"top front right",
    u8"top back left",
    u8"top back center",
    u8"top back right"
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw Nuclex::Support::Errors::CorruptStringError(
        u8"Illegal UTF-8 character(s) encountered"
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compares two string_views containing UTF-8, ignoring casing</summary>
  /// <param name="left">String view that will be compared on the left side</param>
  /// <param name="right">String view that will be compared on the right side</param>
  /// <returns>True if the two string views are identical, ignoring case</returns>
  bool areStringViewsEqual(const std::string_view &left, const std::string_view &right) {
    using Nuclex::Support::Text::UnicodeHelper;

    const UnicodeHelper::Char8Type *haystack, *haystackEnd, *needle, *needleEnd;
    {
      std::string_view::size_type leftLength = left.length();
      std::string_view::size_type rightLength = right.length();
      if(leftLength != rightLength) {
        return false;
      }

      haystack = reinterpret_cast<const UnicodeHelper::Char8Type *>(left.data());
      needle = reinterpret_cast<const UnicodeHelper::Char8Type *>(right.data());
      haystackEnd = haystack + leftLength;
      needleEnd = needle + rightLength;
    }

    for(;;) {
      if(needle >= needleEnd) {
        return (haystack >= haystackEnd); // Both must end at the same time
      }
      if(haystack >= haystackEnd) {
        return false; // If the haystack was shorter, the needle wasn't found
      }

      // Fetch the next code points from both strings so we can compare them
      char32_t haystackCodePoint = UnicodeHelper::ReadCodePoint(haystack, haystackEnd);
      requireValidCodePoint(haystackCodePoint);
      char32_t needleCodePoint = UnicodeHelper::ReadCodePoint(needle, needleEnd);
      requireValidCodePoint(needleCodePoint);

      needleCodePoint = UnicodeHelper::ToFoldedLowercase(needleCodePoint);
      haystackCodePoint = UnicodeHelper::ToFoldedLowercase(haystackCodePoint);

      if(needleCodePoint != haystackCodePoint) {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Parses the channel placement from its textual representation</summary>
  /// <param name="text">Text from which  the channel placement will be parsed</param>
  /// <returns>The channel placement specified in the string</returns>
  Nuclex::Audio::ChannelPlacement identifyChannel(const std::string_view &text) {
    using Nuclex::Support::Text::ParserHelper;

    // Will be set when a word is encountered that matches any of the channel tags
    bool isNone = false;
    bool isLeft = false, isRight = false, isCenter = false;
    bool isFront = false, isBack = false;
    bool isTop = false, isBottom = false;
    bool isBass = false, isLfe = false, isLow = false, isFrequency = false, isEffects = false;

    const ParserHelper::Char8Type *start = (
      reinterpret_cast<const ParserHelper::Char8Type *>(text.data())
    );
    const ParserHelper::Char8Type *end = start + text.length();

    // Check each word in the textual placement against the tags we use to parse
    // the intended channel placement
    std::string_view word;
    for(;;) {
      ParserHelper::FindWord(start, end, &word);
      if(word.empty()) {
        break;
      }

      // Check the word against each of the channel tags. We current don't check
      // if the word is a complete mismatch, so channel names containing words we
      // don't understand are parsed by ignoring those words.
      isNone |= areStringViewsEqual(word, NoneTag);
      isLeft |= areStringViewsEqual(word, LeftTag);
      isRight |= areStringViewsEqual(word, RightTag);
      isCenter |= areStringViewsEqual(word, CenterTag);
      isFront |= areStringViewsEqual(word, FrontTag);
      isBack |= areStringViewsEqual(word, BackTag);
      isBack |= areStringViewsEqual(word, RearTag);
      isTop |= areStringViewsEqual(word, TopTag);
      isBottom |= areStringViewsEqual(word, BottomTag);
      isBass |= areStringViewsEqual(word, BassTag);
      isLfe |= areStringViewsEqual(word, LfeTag);
      isLow |= areStringViewsEqual(word, LowTag);
      isFrequency |= areStringViewsEqual(word, FrequencyTag);
      isEffects |= areStringViewsEqual(word, EffectsTag);

      start += word.length();
    }

    // Certain tag combinations are invalid. That includes combining left and right,
    // front and back or top and bottom. The LFE channel also only be specified alone
    // and not with any directional placement.
    bool isInvalid = (
      (isLeft && isRight) ||
      (isFront && isBack) ||
      (isBottom && isTop)
    );
    isInvalid |= (
      (isLeft || isCenter || isRight || isFront || isBack || isBottom || isTop) &&
      (isBass || isLfe || isLow || isFrequency || isEffects)
    );
    isInvalid |= (
      isNone &&
      (
        isLeft || isCenter || isRight || isFront || isBack || isBottom || isTop ||
        isBass || isLfe || isLow || isFrequency || isEffects
      )
    );

    // This could be a really easy task if each direction tag had its own bit
    // in the channel placement mask, but there is prior art: Microsoft's wave files
    // have a channel mask field and those have become the de-facto standard for channel
    // ordering. The funky tree of ifs below is how we conclude the channel placement
    // from the tags used to describe it in text.
    if(!isInvalid) {
      if(isNone) {
        return Nuclex::Audio::ChannelPlacement::Unknown;
      } else if(isTop) {
        if(isFront) {
          if(isLeft) {
            return Nuclex::Audio::ChannelPlacement::TopFrontLeft;
          } else if(isRight) {
            return Nuclex::Audio::ChannelPlacement::TopFrontRight;
          } else {
            return Nuclex::Audio::ChannelPlacement::TopFrontCenter;
          }
        } else if(isBack) {
          if(isLeft) {
            return Nuclex::Audio::ChannelPlacement::TopBackLeft;
          } else if(isRight) {
            return Nuclex::Audio::ChannelPlacement::TopBackRight;
          } else {
            return Nuclex::Audio::ChannelPlacement::TopBackCenter;
          }
        } else {
          if(!isLeft && !isRight) {
            return Nuclex::Audio::ChannelPlacement::TopCenter;
          }
        }
      } else if(isBottom) {
        // We have no bottom placements but the word is reserved,
        // so using it in any context triggers the invalid tag combination error below
      } else if(!isBass && !isLfe && !isLow && !isFrequency && !isEffects) {
        if(isFront) {
          if(isLeft) {
            if(isCenter) {
              return Nuclex::Audio::ChannelPlacement::FrontCenterLeft;
            } else {
              return Nuclex::Audio::ChannelPlacement::FrontLeft;
            }
          } else if(isRight) {
            if(isCenter) {
              return Nuclex::Audio::ChannelPlacement::FrontCenterRight;
            } else {
              return Nuclex::Audio::ChannelPlacement::FrontRight;
            }
          } else {
            return Nuclex::Audio::ChannelPlacement::FrontCenter;
          }
        } else if(isBack) {
          if(isLeft && !isCenter) {
            return Nuclex::Audio::ChannelPlacement::BackLeft;
          } else if(isRight && !isCenter) {
            return Nuclex::Audio::ChannelPlacement::BackRight;
          } else if(!isLeft && !isRight) {
            return Nuclex::Audio::ChannelPlacement::BackCenter;
          }
        } else {
          if(isLeft && !isCenter) {
            return Nuclex::Audio::ChannelPlacement::SideLeft;
          } else if(isRight && !isCenter) {
            return Nuclex::Audio::ChannelPlacement::SideRight;
          }
        }
      } else {
        if(isBass) {
          if(!isLfe && !isLow && !isFrequency && !isEffects) {
            return Nuclex::Audio::ChannelPlacement::LowFrequencyEffects;
          }
        } else if(isLfe) {
          if(!isLow && !isFrequency && !isEffects) {
            return Nuclex::Audio::ChannelPlacement::LowFrequencyEffects;
          }
        } else if(isLow) {
          if(isFrequency) {
            return Nuclex::Audio::ChannelPlacement::LowFrequencyEffects;
          }
        }
      }
    }

    // If this point is reached, the combination of tags was invalid
    {
      std::string message(u8"Invalid channel tag combination: ", 33);
      message.append(text);
      throw std::invalid_argument(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Audio {

  // ------------------------------------------------------------------------------------------- //

  std::string StringFromChannelPlacement(ChannelPlacement placement) {
    std::string result;
    {
      std::size_t channelCount = Nuclex::Support::BitTricks::CountBits(
        static_cast<std::size_t>(placement) & 0x3FFFF
      );
      result.reserve(channelCount * 12); // channel names average 12 characters
    }

    for(std::size_t bitIndex = 0; bitIndex < 17; ++bitIndex) {
      if((static_cast<std::size_t>(placement) & (1ULL << bitIndex)) != 0) {
        if(result.empty()) {
          result.append(channelNames[bitIndex]);
        } else {
          result.append(u8", ", 2);
          result.append(channelNames[bitIndex]);
        }
      }
    }

    if(result.empty()) {
      result.assign(u8"none", 4);
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  ChannelPlacement ChannelPlacementFromString(const std::string &channelPlacementAsText) {
    ChannelPlacement result = ChannelPlacement::Unknown;

    std::string_view text(channelPlacementAsText);
    std::string_view::size_type startIndex = 0;
    for(;;) {
      std::string_view::size_type endIndex = text.find(u8',', startIndex);
      if(endIndex == std::string_view::npos) {
        endIndex = text.length();
        if(startIndex < endIndex) {
          result = result | identifyChannel(text.substr(startIndex, endIndex - startIndex));
        }
        break;
      }

      result = result | identifyChannel(text.substr(startIndex, endIndex - startIndex));
      startIndex = endIndex + 1;
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Audio
