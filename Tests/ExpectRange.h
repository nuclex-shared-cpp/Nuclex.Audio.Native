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

#ifndef NUCLEX_AUDIO_EXPECTRANGE_H
#define NUCLEX_AUDIO_EXPECTRANGE_H

#include "Nuclex/Audio/Config.h"

#include <gtest/gtest.h>

// --------------------------------------------------------------------------------------------- //

// Checks is in the range from lower (inclusive) to upper (exclusive)
//
// This is a little nonstandard GoogleTest macro because such checks appear quite often
// where audio signals are analyzed in the unit tests
#define EXPECT_RANGE(value, lower, upper) \
  EXPECT_PRED_FORMAT3(AssertInRange, value, lower, upper)

// --------------------------------------------------------------------------------------------- //

/// <summary>
///   Fails a GoogleTest expectation if a value falls outside the specified range
/// </summary>
/// <typeparam name="TScalar">Scalar type that will be checked being in-range</typeparam>
/// <param name="valueExpression">Stringified expression used to state the value</param>
/// <param name="lowerExpression">Stringified expression used to state the lower bound</param>
/// <param name="upperExpression">Stringified expression used to state the upper bound</param>
/// <param name="value">Value that will be compared</param>
/// <param name="lower">Minimum value that is allowed (inclusive)</param>
/// <param name="upper">Maximum value that is allowed (exclusive)</param>
/// <returns>The result of the GoogleTest assertion</returns>
template<typename TScalar>
inline ::testing::AssertionResult AssertInRange(
  const char *valueExpression, const char *lowerExpression, const char *upperExpression,
  TScalar value, TScalar lower, TScalar upper
) {
  if(value >= lower && value < upper) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() <<
      valueExpression << u8" (" << value << u8") is not in range " <<
      u8"[" << lowerExpression << u8" (" << lower << u8"), " <<
      upperExpression << u8" (" << upper << u8"))";
  }
}

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_AUDIO_EXPECTRANGE_H