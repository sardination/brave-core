// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/brave_today/browser/unsupported_publisher_migrator.h"
#include <string>
#include <utility>
#include "base/bind.h"
#include "base/containers/flat_map.h"
#include "brave/components/api_request_helper/api_request_helper.h"
#include "brave/components/brave_private_cdn/headers.h"
#include "brave/components/brave_today/browser/direct_feed_controller.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "brave/components/brave_today/browser/publishers_parsing.h"
#include "brave/components/brave_today/browser/urls.h"
#include "brave/components/brave_today/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "url/gurl.h"

namespace brave_news {

UnsupportedPublisherMigrator::UnsupportedPublisherMigrator(
    PrefService* prefs,
    DirectFeedController* direct_feed_controller,
    api_request_helper::APIRequestHelper* api_request_helper)
    : prefs_(prefs),
      direct_feed_controller_(direct_feed_controller),
      api_request_helper_(api_request_helper) {
  Init();
}

UnsupportedPublisherMigrator::~UnsupportedPublisherMigrator() = default;

void UnsupportedPublisherMigrator::Init() {
  GURL sources_url("https://" + brave_today::GetHostname() + "/sources." +
                   brave_today::GetRegionUrlPart() + "json");
  auto onResponse = base::BindOnce(
      [](UnsupportedPublisherMigrator* migrator, const int status,
         const std::string& body,
         const base::flat_map<std::string, std::string>& headers) {
        VLOG(1) << "Downloaded old sources, status: " << status;

        // We didn't manage to receive the old sources. Don't do anything else
        // and if we get a response next time, we'll migrate then.
        if (status < 200 || status >= 300)
          return;

        ParseCombinedPublisherList(body, &migrator->v1_api_publishers_);
        migrator->on_init_complete_->Signal();
      },
      base::Unretained(this));

  api_request_helper_->Request("GET", sources_url, "", "", true,
                               std::move(onResponse),
                               brave::private_cdn_headers);
}

void UnsupportedPublisherMigrator::MaybeAddUnsupportedPublisherToDirectFeeds(
    const std::string& publisher_id) {
  if (on_init_complete_->is_signaled()) {
    on_init_complete_->Post(
        FROM_HERE, base::BindOnce(&UnsupportedPublisherMigrator::
                                      MaybeAddUnsupportedPublisherToDirectFeeds,
                                  base::Unretained(this), publisher_id));
    return;
  }

  auto it = v1_api_publishers_.find(publisher_id);

  if (it == v1_api_publishers_.end()) {
    VLOG(1) << "Encountered unknown publisher id: " << publisher_id
            << " which wasn't removed in the migration to the v2 API";
    return;
  }

  // As we found a match, add it as a direct feed. This may fail if the feed
  // already exists, but that's fine (because it will still show up).
  direct_feed_controller_->AddDirectFeed(
      it->second->feed_source, it->second->publisher_name, publisher_id);

  // Once we've added the direct feed, delete the feed from our combined
  // publishers list.
  DictionaryPrefUpdate update(prefs_, prefs::kBraveTodaySources);
  update->RemoveKey(publisher_id);
}

}  // namespace brave_news
