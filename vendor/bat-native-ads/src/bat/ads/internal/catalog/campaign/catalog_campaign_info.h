/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CATALOG_CAMPAIGN_CATALOG_CAMPAIGN_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CATALOG_CAMPAIGN_CATALOG_CAMPAIGN_INFO_H_

#include <string>
#include <vector>

#include "bat/ads/internal/catalog/campaign/catalog_daypart_info.h"
#include "bat/ads/internal/catalog/campaign/catalog_geo_target_info.h"
#include "bat/ads/internal/catalog/campaign/creative_set/catalog_creative_set_info.h"

namespace ads {

struct CatalogCampaignInfo final {
  CatalogCampaignInfo();
  CatalogCampaignInfo(const CatalogCampaignInfo& info);
  CatalogCampaignInfo& operator=(const CatalogCampaignInfo& info);
  ~CatalogCampaignInfo();

  bool operator==(const CatalogCampaignInfo& rhs) const;
  bool operator!=(const CatalogCampaignInfo& rhs) const;

  std::string campaign_id;
  unsigned int priority = 0;
  double ptr = 0.0;
  std::string start_at;
  std::string end_at;
  unsigned int daily_cap = 0;
  std::string advertiser_id;
  CatalogCreativeSetList creative_sets;
  CatalogDaypartList dayparts;
  CatalogGeoTargetList geo_targets;
};

using CatalogCampaignList = std::vector<CatalogCampaignInfo>;

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CATALOG_CAMPAIGN_CATALOG_CAMPAIGN_INFO_H_
