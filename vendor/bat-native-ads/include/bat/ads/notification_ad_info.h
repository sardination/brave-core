/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_INCLUDE_BAT_ADS_NOTIFICATION_AD_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_INCLUDE_BAT_ADS_NOTIFICATION_AD_INFO_H_

#include <string>

#include "base/values.h"
#include "bat/ads/ad_info.h"
#include "bat/ads/export.h"

namespace ads {

struct ADS_EXPORT NotificationAdInfo final : AdInfo {
  NotificationAdInfo();
  NotificationAdInfo(const NotificationAdInfo& info);
  NotificationAdInfo& operator=(const NotificationAdInfo& info);
  ~NotificationAdInfo();

  bool IsValid() const;

  base::Value::Dict ToValue() const;

  std::string ToJson() const;
  bool FromJson(const std::string& json);
  void FromValue(const base::Value::Dict& value);

  std::string title;
  std::string body;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_INCLUDE_BAT_ADS_NOTIFICATION_AD_INFO_H_
