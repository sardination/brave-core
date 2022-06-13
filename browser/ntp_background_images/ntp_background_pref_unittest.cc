/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ntp_background_images/ntp_background_pref.h"

#include "brave/components/constants/pref_names.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "testing/gtest/include/gtest/gtest.h"

class NTPBackgroundPrefTest : public testing::Test {
 public:
  NTPBackgroundPrefTest() {
    NTPBackgroundPref::RegisterPref(service_.registry());
  }

  sync_preferences::TestingPrefServiceSyncable service_;
  NTPBackgroundPref background_pref_{&service_};
};

TEST_F(NTPBackgroundPrefTest, RegisterDefaultPref) {
  const auto* value = service_.GetDictionary(NTPBackgroundPref::kPrefName);
  EXPECT_TRUE(value);

  const auto* dict = value->GetIfDict();
  EXPECT_TRUE(dict->FindString("type"));
  EXPECT_TRUE(dict->FindBool("random").has_value());
  EXPECT_TRUE(dict->FindString("selected_value"));
}

TEST_F(NTPBackgroundPrefTest, TypeAccessor) {
  EXPECT_TRUE(background_pref_.IsBraveType());

  background_pref_.SetType(NTPBackgroundPref::Type::kCustomImage);
  EXPECT_TRUE(background_pref_.IsCustomImageType());

  background_pref_.SetType(NTPBackgroundPref::Type::kSolidColor);
  EXPECT_TRUE(background_pref_.IsSolidColorType());

  background_pref_.SetType(NTPBackgroundPref::Type::kGradientColor);
  EXPECT_TRUE(background_pref_.IsGradientColorType());
}

TEST_F(NTPBackgroundPrefTest, MigrationTest) {
  auto* registry = service_.registry();
  registry->RegisterBooleanPref(NTPBackgroundPref::kDeprecatedPrefName, false);
  EXPECT_FALSE(service_.GetBoolean(NTPBackgroundPref::kDeprecatedPrefName));
  EXPECT_TRUE(service_.GetDictionary(NTPBackgroundPref::kPrefName));

  // Check default value
  EXPECT_TRUE(background_pref_.IsBraveType());

  // Check if migration does nothing when custom background was not enabled.
  background_pref_.MigrateOldPref();
  EXPECT_TRUE(background_pref_.IsBraveType());

  // Check if migration works properly.
  service_.SetBoolean(NTPBackgroundPref::kDeprecatedPrefName, true);
  background_pref_.MigrateOldPref();
  EXPECT_TRUE(background_pref_.IsCustomImageType());
  EXPECT_FALSE(service_.GetBoolean(NTPBackgroundPref::kDeprecatedPrefName));
}

TEST_F(NTPBackgroundPrefTest, SelectedValue) {
  EXPECT_TRUE(background_pref_.IsBraveType());

  constexpr char kSelectedURL[] = "http://selected.com/img.jpg";
  background_pref_.SetSelectedValue(kSelectedURL);
  auto selected_value = background_pref_.GetSelectedValue();
  EXPECT_TRUE(absl::holds_alternative<GURL>(selected_value));
  EXPECT_TRUE(absl::get<GURL>(selected_value).spec() == kSelectedURL);

  background_pref_.SetType(NTPBackgroundPref::Type::kCustomImage);
  selected_value = background_pref_.GetSelectedValue();
  EXPECT_TRUE(absl::holds_alternative<GURL>(selected_value));

  background_pref_.SetType(NTPBackgroundPref::Type::kSolidColor);
  background_pref_.SetSelectedValue("red");
  selected_value = background_pref_.GetSelectedValue();
  EXPECT_TRUE(absl::holds_alternative<std::string>(selected_value));
  EXPECT_TRUE(absl::get<std::string>(selected_value) == "red");

  background_pref_.SetType(NTPBackgroundPref::Type::kGradientColor);
  selected_value = background_pref_.GetSelectedValue();
  EXPECT_TRUE(absl::holds_alternative<std::string>(selected_value));
}
