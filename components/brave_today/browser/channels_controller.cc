// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/brave_today/browser/channels_controller.h"

#include <utility>

#include "base/bind.h"
#include "base/containers/contains.h"
#include "base/containers/flat_map.h"
#include "brave/components/brave_today/browser/brave_news_controller.h"
#include "brave/components/brave_today/browser/publishers_controller.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"
#include "brave/components/brave_today/common/brave_news.mojom.h"
#include "brave/components/brave_today/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"

namespace brave_news {
ChannelsController::ChannelsController(
    PrefService* prefs,
    PublishersController* publishers_controller)
    : prefs_(prefs), publishers_controller_(publishers_controller) {}

ChannelsController::~ChannelsController() = default;

Channels ChannelsController::GetChannelsFromPublishers(
    const Publishers& publishers,
    PrefService* prefs) {
  Channels channels;
  auto* subscriptions = prefs->GetDictionary(prefs::kBraveNewsSubscriptions);

  for (const auto& it : publishers) {
    for (const auto& channel_id : it.second->channels) {
      // Don't add channels which already exist.
      if (base::Contains(channels, channel_id))
        continue;

      auto channel = mojom::Channel::New();
      channel->channel_name = channel_id;
      channel->subscribed =
          subscriptions->FindBoolKey(channel_id).value_or(false);

      channels.insert({channel_id, std::move(channel)});
    }
  }
  return channels;
}

void ChannelsController::GetAllChannels(ChannelsCallback callback) {
  publishers_controller_->GetOrFetchPublishers(base::BindOnce(
      [](ChannelsCallback callback, PrefService* prefs, Publishers publishers) {
        auto result = GetChannelsFromPublishers(publishers, prefs);
        std::move(callback).Run(std::move(std::move(result)));
      },
      std::move(callback), base::Unretained(prefs_)));
}

mojom::ChannelPtr ChannelsController::SetChannelSubscribed(
    const std::string& channel_id,
    bool subscribed) {
  DictionaryPrefUpdate update(prefs_, prefs::kBraveNewsSubscriptions);
  update->SetBoolKey(channel_id, subscribed);

  auto result = mojom::Channel::New();
  result->channel_name = channel_id;
  result->subscribed = subscribed;
  return result;
}

bool ChannelsController::GetChannelSubscribed(const std::string& channel_id) {
  auto* subscriptions = prefs_->GetDictionary(prefs::kBraveNewsSubscriptions);
  return subscriptions->FindBoolKey(channel_id).value_or(false);
}
}  // namespace brave_news
