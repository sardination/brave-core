/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/privacy/tokens/token_generator.h"

#include "bat/ads/internal/privacy/challenge_bypass_ristretto/token.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace privacy {

TEST(BatAdsTokensTest, Generate) {
  // Arrange
  TokenGenerator token_generator;

  // Act
  const std::vector<cbr::Token> tokens = token_generator.Generate(5);

  // Assert
  const size_t count = tokens.size();
  EXPECT_EQ(5UL, count);
}

TEST(BatAdsTokensTest, GenerateZero) {
  // Arrange
  TokenGenerator token_generator;

  // Act
  const std::vector<cbr::Token> tokens = token_generator.Generate(0);

  // Assert
  EXPECT_TRUE(tokens.empty());
}

}  // namespace privacy
}  // namespace ads
