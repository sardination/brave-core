/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TABS_TAB_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TABS_TAB_INFO_H_

#include <cstdint>

#include "url/gurl.h"

namespace ads {

struct TabInfo final {
  TabInfo();
  TabInfo(const TabInfo& info);
  TabInfo& operator=(const TabInfo& info);
  ~TabInfo();

  bool operator==(const TabInfo& rhs) const;
  bool operator!=(const TabInfo& rhs) const;

  int32_t id = 0;
  GURL url;
  bool is_playing_media = false;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TABS_TAB_INFO_H_
