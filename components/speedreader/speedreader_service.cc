/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/speedreader/speedreader_service.h"

#include "base/feature_list.h"
#include "base/metrics/histogram_macros.h"
#include "brave/components/speedreader/common/features.h"
#include "brave/components/speedreader/speedreader_pref_names.h"
#include "brave/components/time_period_storage/weekly_storage.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"

namespace speedreader {

namespace {

// Note: append-only array! Never remove any existing values, as this array
// is used to bucket a UMA histogram, and removing values breaks that.
constexpr std::array<uint64_t, 5> kSpeedReaderToggleBuckets{
    0,   // 0
    5,   // >0-5
    10,  // 6-10
    20,  // 11-20
    30,  // 21-30
         // 30+ => bucket 5
};

// Note: append-only enumeration! Never remove any existing values, as this enum
// is used to bucket a UMA histogram, and removing values breaks that.
enum class EnabledStatus {
  kUnused,
  kEverEnabled,
  kRecentlyUsed,
  kMaxValue = kRecentlyUsed
};

constexpr char kSpeedreaderToggleUMAHistogramName[] =
    "Brave.SpeedReader.ToggleCount";

constexpr char kSpeedreaderEnabledUMAHistogramName[] =
    "Brave.SpeedReader.Enabled";

void StoreTogglesHistogram(uint64_t toggles) {
  int bucket = 0;
  for (const auto& bucket_upper_bound : kSpeedReaderToggleBuckets) {
    if (toggles > bucket_upper_bound)
      bucket = bucket + 1;
  }

  UMA_HISTOGRAM_EXACT_LINEAR(kSpeedreaderToggleUMAHistogramName, bucket, 5);
}

void RecordHistograms(PrefService* prefs, bool toggled, bool enabled_now) {
  WeeklyStorage weekly_toggles(prefs, kSpeedreaderPrefToggleCount);
  if (toggled)
    weekly_toggles.AddDelta(1);
  const uint64_t toggle_count = weekly_toggles.GetWeeklySum();
  StoreTogglesHistogram(toggle_count);

  // Has been "recently" enabled if currently enabled,
  // or got disabled this week.
  EnabledStatus status = EnabledStatus::kUnused;
  if (enabled_now || toggle_count > 0) {
    status = EnabledStatus::kRecentlyUsed;
  } else if (prefs->GetBoolean(kSpeedreaderPrefEverEnabled)) {
    status = EnabledStatus::kEverEnabled;
  }

  UMA_HISTOGRAM_ENUMERATION(kSpeedreaderEnabledUMAHistogramName, status);
}

}  // namespace

SpeedreaderService::SpeedreaderService(PrefService* prefs) : prefs_(prefs) {}

SpeedreaderService::~SpeedreaderService() = default;

// static
void SpeedreaderService::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kSpeedreaderPrefEnabled, false);
  registry->RegisterBooleanPref(kSpeedreaderPrefEverEnabled, false);
  registry->RegisterListPref(kSpeedreaderPrefToggleCount);
  registry->RegisterIntegerPref(kSpeedreaderPrefPromptCount, 0);
  registry->RegisterIntegerPref(kSpeedreaderPrefTheme,
                                static_cast<int>(Theme::kNone));
}

void SpeedreaderService::ToggleSpeedreader() {
  const bool toggled_value = !prefs_->GetBoolean(kSpeedreaderPrefEnabled);
  prefs_->SetBoolean(kSpeedreaderPrefEnabled, toggled_value);
  if (toggled_value)
    prefs_->SetBoolean(kSpeedreaderPrefEverEnabled, true);
  RecordHistograms(prefs_, true, toggled_value);
}

void SpeedreaderService::DisableSpeedreaderForTest() {
  prefs_->SetBoolean(kSpeedreaderPrefEnabled, false);
  prefs_->SetBoolean(kSpeedreaderPrefEverEnabled, false);
  prefs_->SetInteger(kSpeedreaderPrefPromptCount, 0);
}

bool SpeedreaderService::IsEnabled() {
  if (!base::FeatureList::IsEnabled(kSpeedreaderFeature)) {
    return false;
  }

  const bool enabled = prefs_->GetBoolean(kSpeedreaderPrefEnabled);
  RecordHistograms(prefs_, false, enabled);
  return enabled;
}

bool SpeedreaderService::ShouldPromptUserToEnable() const {
  constexpr int max_speedreader_prompts = 2;

  if (prefs_->GetBoolean(kSpeedreaderPrefEverEnabled) ||
      prefs_->GetInteger(kSpeedreaderPrefPromptCount) >
          max_speedreader_prompts) {
    return false;
  }
  return true;
}

void SpeedreaderService::IncrementPromptCount() {
  const int count = prefs_->GetInteger(kSpeedreaderPrefPromptCount);
  prefs_->SetInteger(kSpeedreaderPrefPromptCount, count + 1);
}

void SpeedreaderService::SetTheme(Theme theme) {
  prefs_->SetInteger(kSpeedreaderPrefTheme, static_cast<int>(theme));
}

Theme SpeedreaderService::GetTheme() const {
  return static_cast<Theme>(prefs_->GetInteger(kSpeedreaderPrefTheme));
}

std::string SpeedreaderService::GetThemeName() const {
  switch (GetTheme()) {
    default:
      return {};
    case Theme::kNone:
      return {};
    case Theme::kLight:
      return "light";
    case Theme::kSepia:
      return "sepia";
    case Theme::kDark:
      return "dark";
  }
}

}  // namespace speedreader
