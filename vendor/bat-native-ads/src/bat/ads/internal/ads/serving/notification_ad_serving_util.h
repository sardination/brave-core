/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_UTIL_H_

namespace base {
class Time;
class TimeDelta;
}  // namespace base

namespace ads {
namespace notification_ads {

bool ShouldServeAdsAtRegularIntervals();

base::TimeDelta CalculateDelayBeforeServingAnAd();
base::Time ServeAdAt();
void SetServeAdAt(const base::Time serve_ad_at);

}  // namespace notification_ads
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_UTIL_H_
