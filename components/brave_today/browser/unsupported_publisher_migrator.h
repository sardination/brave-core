// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_UNSUPPORTED_PUBLISHER_MIGRATOR_H_
#define BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_UNSUPPORTED_PUBLISHER_MIGRATOR_H_

#include <memory>
#include <string>
#include "base/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/one_shot_event.h"
#include "base/timer/timer.h"
#include "brave/components/api_request_helper/api_request_helper.h"
#include "brave/components/brave_today/browser/direct_feed_controller.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "components/prefs/pref_service.h"

namespace brave_news {

class UnsupportedPublisherMigrator {
 public:
  UnsupportedPublisherMigrator(
      PrefService* prefs,
      DirectFeedController* direct_feed_controller,
      api_request_helper::APIRequestHelper* api_request_helper);
  ~UnsupportedPublisherMigrator();
  UnsupportedPublisherMigrator(const UnsupportedPublisherMigrator&) = delete;
  UnsupportedPublisherMigrator& operator=(const UnsupportedPublisherMigrator&) =
      delete;

  void MaybeAddUnsupportedPublisherToDirectFeeds(
      const std::string& publisher_id);

 private:
  void WhenPublishersReceived(base::OnceClosure when_received_);
  void Init();

  raw_ptr<PrefService> prefs_;
  raw_ptr<DirectFeedController> direct_feed_controller_;
  raw_ptr<api_request_helper::APIRequestHelper> api_request_helper_;

  Publishers v1_api_publishers_;
  std::unique_ptr<base::OneShotEvent> on_init_complete_;
};

}  // namespace brave_news

#endif  // BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_UNSUPPORTED_PUBLISHER_MIGRATOR_H_
