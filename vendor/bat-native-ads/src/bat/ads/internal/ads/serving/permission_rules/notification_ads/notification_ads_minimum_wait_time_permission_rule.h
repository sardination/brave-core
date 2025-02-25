/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_NOTIFICATION_ADS_NOTIFICATION_ADS_MINIMUM_WAIT_TIME_PERMISSION_RULE_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_NOTIFICATION_ADS_NOTIFICATION_ADS_MINIMUM_WAIT_TIME_PERMISSION_RULE_H_

#include <string>
#include <vector>

#include "bat/ads/internal/ads/serving/permission_rules/permission_rule_interface.h"

namespace base {
class Time;
}  // namespace base

namespace ads {
namespace notification_ads {

class MinimumWaitTimePermissionRule final : public PermissionRuleInterface {
 public:
  MinimumWaitTimePermissionRule();
  ~MinimumWaitTimePermissionRule() override;

  bool ShouldAllow() override;

  std::string GetLastMessage() const override;

 private:
  bool DoesRespectCap(const std::vector<base::Time>& history);

  MinimumWaitTimePermissionRule(const MinimumWaitTimePermissionRule&) = delete;
  MinimumWaitTimePermissionRule& operator=(
      const MinimumWaitTimePermissionRule&) = delete;

  std::string last_message_;
};

}  // namespace notification_ads
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_SERVING_PERMISSION_RULES_NOTIFICATION_ADS_NOTIFICATION_ADS_MINIMUM_WAIT_TIME_PERMISSION_RULE_H_
