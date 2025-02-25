/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_

#include "base/feature_list.h"

namespace ads {
namespace permission_rules {
namespace features {

extern const base::Feature kFeature;

bool IsEnabled();

bool ShouldOnlyServeAdsInWindowedMode();

bool ShouldOnlyServeAdsWithValidInternetConnection();

bool ShouldOnlyServeAdsIfMediaIsNotPlaying();

bool ShouldOnlyServeAdsIfBrowserIsActive();

}  // namespace features
}  // namespace permission_rules
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_
