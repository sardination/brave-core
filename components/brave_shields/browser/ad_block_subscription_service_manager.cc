/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/base64url.h"
#include "base/bind.h"
#include "base/containers/contains.h"
#include "base/files/file_util.h"
#include "base/json/json_value_converter.h"
#include "base/json/values_util.h"
#include "base/strings/string_util.h"
#include "base/task/thread_pool.h"
#include "base/thread_annotations.h"
#include "base/time/time.h"
#include "base/values.h"
#include "brave/components/adblock_rust_ffi/src/wrapper.h"
#include "brave/components/brave_shields/browser/ad_block_engine.h"
#include "brave/components/brave_shields/browser/ad_block_service_helper.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_filters_provider.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager_observer.h"
#include "brave/components/brave_shields/common/brave_shield_constants.h"
#include "brave/components/brave_shields/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "content/public/browser/browser_task_traits.h"
#include "crypto/sha2.h"
#include "net/base/filename_util.h"

namespace brave_shields {

base::TimeDelta* g_testing_subscription_retry_interval;

namespace {

bool SkipGURLField(base::StringPiece value, GURL* field) {
  return true;
}

bool ParseTimeValue(const base::Value* value, base::Time* field) {
  auto time = base::ValueToTime(value);
  if (!time) {
    return false;
  }
  *field = *time;
  return true;
}

bool ParseOptionalStringField(const base::Value* value,
                              absl::optional<std::string>* field) {
  if (value == nullptr) {
    *field = absl::nullopt;
    return true;
  } else if (!value->is_string()) {
    return false;
  } else {
    *field = value->GetString();
    return true;
  }
}

const base::TimeDelta kListUpdateInterval = base::Days(7);
const base::TimeDelta kListRetryInterval = base::Hours(1);
const base::TimeDelta kListCheckInitialDelay = base::Minutes(1);

SubscriptionInfo BuildInfoFromDict(const GURL& sub_url,
                                   const base::Value* dict) {
  DCHECK(dict);
  DCHECK(dict->is_dict());

  SubscriptionInfo info;
  base::JSONValueConverter<SubscriptionInfo> converter;
  converter.Convert(*dict, &info);

  info.subscription_url = sub_url;

  return info;
}

const base::FilePath::CharType kSubscriptionsDir[] =
    FILE_PATH_LITERAL("FilterListSubscriptionCache");

}  // namespace

SubscriptionInfo::SubscriptionInfo() {}
SubscriptionInfo::~SubscriptionInfo() {}
SubscriptionInfo::SubscriptionInfo(const SubscriptionInfo&) = default;

void SubscriptionInfo::RegisterJSONConverter(
    base::JSONValueConverter<SubscriptionInfo>* converter) {
  // The `subscription_url` field is skipped, as it's not stored within the
  // JSON value and should be populated externally.
  converter->RegisterCustomField<GURL>(
      "subscription_url", &SubscriptionInfo::subscription_url, &SkipGURLField);
  converter->RegisterCustomValueField<base::Time>(
      "last_update_attempt", &SubscriptionInfo::last_update_attempt,
      &ParseTimeValue);
  converter->RegisterCustomValueField<base::Time>(
      "last_successful_update_attempt",
      &SubscriptionInfo::last_successful_update_attempt, &ParseTimeValue);
  converter->RegisterBoolField("enabled", &SubscriptionInfo::enabled);
  converter->RegisterCustomValueField<absl::optional<std::string>>(
      "homepage", &SubscriptionInfo::homepage, &ParseOptionalStringField);
  converter->RegisterCustomValueField<absl::optional<std::string>>(
      "title", &SubscriptionInfo::title, &ParseOptionalStringField);
}

AdBlockSubscriptionServiceManager::AdBlockSubscriptionServiceManager(
    PrefService* local_state,
    scoped_refptr<base::SequencedTaskRunner> task_runner,
    AdBlockSubscriptionDownloadManager::DownloadManagerGetter
        download_manager_getter,
    const base::FilePath& profile_dir)
    : initialized_(false),
      local_state_(local_state),
      task_runner_(task_runner),
      subscription_path_(profile_dir.Append(kSubscriptionsDir)),
      subscription_update_timer_(
          std::make_unique<component_updater::TimerUpdateScheduler>()) {
  std::move(download_manager_getter)
      .Run(base::BindOnce(
          &AdBlockSubscriptionServiceManager::OnGetDownloadManager,
          weak_ptr_factory_.GetWeakPtr()));
}

void AdBlockSubscriptionServiceManager::Init(
    AdBlockResourceProvider* resource_provider) {
  resource_provider_ = resource_provider;
  initialized_ = true;
}

bool AdBlockSubscriptionServiceManager::IsInitialized() {
  return initialized_;
}

AdBlockSubscriptionServiceManager::~AdBlockSubscriptionServiceManager() {}

base::FilePath AdBlockSubscriptionServiceManager::GetSubscriptionPath(
    const GURL& sub_url) const {
  // Subdirectories are generated by taking the SHA256 hash of the list URL
  // spec, then base64 encoding that hash. This generates paths that are:
  //     - deterministic
  //     - unique
  //     - constant length
  //     - path-safe
  //     - not too long (exactly 45 characters)
  const std::string hash = crypto::SHA256HashString(sub_url.spec());

  std::string pathsafe_hash;
  base::Base64UrlEncode(hash, base::Base64UrlEncodePolicy::INCLUDE_PADDING,
                        &pathsafe_hash);

  return subscription_path_.AppendASCII(pathsafe_hash);
}

GURL AdBlockSubscriptionServiceManager::GetListTextFileUrl(
    const GURL sub_url) const {
  base::FilePath cached_list_path = GetSubscriptionPath(sub_url).Append(
      brave_shields::kCustomSubscriptionListText);

  const GURL file_url = net::FilePathToFileURL(cached_list_path);

  return file_url;
}

void AdBlockSubscriptionServiceManager::OnUpdateTimer(
    component_updater::TimerUpdateScheduler::OnFinishedCallback on_finished) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!local_state_)
    return;

  base::AutoLock lock(subscription_services_lock_);
  subscriptions_ = local_state_->GetDictionary(prefs::kAdBlockListSubscriptions)
                       ->GetDict()
                       .Clone();

  for (const auto it : subscriptions_) {
    const std::string key = it.first;
    SubscriptionInfo info;
    const base::Value* list_subscription_dict = subscriptions_.Find(key);
    if (list_subscription_dict && list_subscription_dict->is_dict()) {
      GURL sub_url(key);
      info = BuildInfoFromDict(sub_url, list_subscription_dict);

      base::TimeDelta until_next_refresh =
          kListUpdateInterval - (base::Time::Now() - info.last_update_attempt);

      if (info.enabled &&
          ((info.last_update_attempt != info.last_successful_update_attempt) ||
           (until_next_refresh <= base::TimeDelta()))) {
        StartDownload(sub_url, false);
      }
    }
  }

  std::move(on_finished).Run();
}

void AdBlockSubscriptionServiceManager::StartDownload(const GURL& sub_url,
                                                      bool from_ui) {
  // The download manager is tied to the lifetime of the profile, but
  // the AdBlockSubscriptionServiceManager lives as long as the browser process
  if (download_manager_) {
    bool download_service_available =
        download_manager_->IsAvailableForDownloads();
    if (download_service_available) {
      download_manager_->StartDownload(sub_url, from_ui);
    }
  }
}

void AdBlockSubscriptionServiceManager::CreateSubscription(
    const GURL& sub_url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  {
    base::AutoLock lock(subscription_services_lock_);
    if (base::Contains(subscription_services_, sub_url)) {
      return;
    }
  }

  SubscriptionInfo info;
  info.subscription_url = sub_url;
  info.last_update_attempt = base::Time();
  info.last_successful_update_attempt = base::Time();
  info.enabled = true;

  auto subscription_service =
      std::unique_ptr<AdBlockEngine, base::OnTaskRunnerDeleter>(
          new AdBlockEngine(), base::OnTaskRunnerDeleter(task_runner_));
  UpdateSubscriptionPrefs(sub_url, info);

  auto subscription_filters_provider =
      std::make_unique<AdBlockSubscriptionFiltersProvider>(
          local_state_,
          GetSubscriptionPath(sub_url).Append(kCustomSubscriptionListText));
  auto observer = std::make_unique<AdBlockService::SourceProviderObserver>(
      subscription_service->AsWeakPtr(), subscription_filters_provider.get(),
      resource_provider_, task_runner_,
      base::BindRepeating(&AdBlockSubscriptionServiceManager::OnListMetadata,
                          weak_ptr_factory_.GetWeakPtr(), sub_url));

  {
    base::AutoLock lock(subscription_services_lock_);
    // this could allow more than one service for a given url
    subscription_services_.insert(
        std::make_pair(sub_url, std::move(subscription_service)));
    subscription_filters_providers_.insert(
        std::make_pair(sub_url, std::move(subscription_filters_provider)));
    subscription_source_observers_.insert(
        std::make_pair(sub_url, std::move(observer)));
  }

  StartDownload(sub_url, true);
}

std::vector<SubscriptionInfo>
AdBlockSubscriptionServiceManager::GetSubscriptions() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::AutoLock lock(subscription_services_lock_);

  auto infos = std::vector<SubscriptionInfo>();

  for (const auto& subscription_service : subscription_services_) {
    auto info = GetInfo(subscriptions_, subscription_service.first);
    DCHECK(info);
    infos.push_back(*info);
  }

  return infos;
}

void AdBlockSubscriptionServiceManager::EnableSubscription(const GURL& sub_url,
                                                           bool enabled) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  absl::optional<SubscriptionInfo> info;
  {
    base::AutoLock lock(subscription_services_lock_);
    info = GetInfo(subscriptions_, sub_url);
  }

  DCHECK(info);

  info->enabled = enabled;

  UpdateSubscriptionPrefs(sub_url, *info);
}

void AdBlockSubscriptionServiceManager::DeleteSubscription(
    const GURL& sub_url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  {
    base::AutoLock lock(subscription_services_lock_);
    auto observer = subscription_source_observers_.find(sub_url);
    DCHECK(observer != subscription_source_observers_.end());
    subscription_source_observers_.erase(observer);
    auto it = subscription_services_.find(sub_url);
    DCHECK(it != subscription_services_.end());
    subscription_services_.erase(it);
    auto it2 = subscription_filters_providers_.find(sub_url);
    DCHECK(it2 != subscription_filters_providers_.end());
    subscription_filters_providers_.erase(it2);
  }
  ClearSubscriptionPrefs(sub_url);

  base::ThreadPool::PostTask(
      FROM_HERE,
      {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
       base::TaskShutdownBehavior::BLOCK_SHUTDOWN},
      base::BindOnce(base::IgnoreResult(&base::DeletePathRecursively),
                     GetSubscriptionPath(sub_url)));
}

void AdBlockSubscriptionServiceManager::RefreshSubscription(const GURL& sub_url,
                                                            bool from_ui) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  StartDownload(sub_url, true);
}

void AdBlockSubscriptionServiceManager::OnGetDownloadManager(
    AdBlockSubscriptionDownloadManager* download_manager) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  download_manager_ = download_manager->AsWeakPtr();
  // base::Unretained is ok here because AdBlockSubscriptionServiceManager will
  // outlive AdBlockSubscriptionDownloadManager
  download_manager_->set_subscription_path_callback(base::BindRepeating(
      &AdBlockSubscriptionServiceManager::GetSubscriptionPath,
      base::Unretained(this)));
  download_manager_->set_on_download_succeeded_callback(base::BindRepeating(
      &AdBlockSubscriptionServiceManager::OnSubscriptionDownloaded,
      base::Unretained(this)));
  download_manager_->set_on_download_failed_callback(base::BindRepeating(
      &AdBlockSubscriptionServiceManager::OnSubscriptionDownloadFailure,
      base::Unretained(this)));

  download_manager_->CancelAllPendingDownloads();
  LoadSubscriptionServices();

  subscription_update_timer_->Schedule(
      kListCheckInitialDelay, kListRetryInterval,
      base::BindRepeating(&AdBlockSubscriptionServiceManager::OnUpdateTimer,
                          weak_ptr_factory_.GetWeakPtr()),
      base::DoNothing());
}

void AdBlockSubscriptionServiceManager::OnListMetadata(
    const GURL& sub_url,
    const adblock::FilterListMetadata& metadata) {
  // The engine will have loaded new list metadata; read it and update local
  // preferences with the new values.

  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  absl::optional<SubscriptionInfo> info;
  {
    base::AutoLock lock(subscription_services_lock_);
    info = GetInfo(subscriptions_, sub_url);
  }

  if (!info)
    return;

  // Title can only be set once - only set it if an existing title does not
  // exist
  if (!info->title && metadata.title) {
    info->title = absl::make_optional(*metadata.title);
  }

  if (metadata.homepage) {
    info->homepage = absl::make_optional(*metadata.homepage);
  } else {
    info->homepage = absl::nullopt;
  }

  UpdateSubscriptionPrefs(sub_url, *info);

  NotifyObserversOfServiceEvent();
}

void AdBlockSubscriptionServiceManager::SetUpdateIntervalsForTesting(
    base::TimeDelta* initial_delay,
    base::TimeDelta* retry_interval) {
  g_testing_subscription_retry_interval = retry_interval;
  subscription_update_timer_->Schedule(
      *initial_delay, *retry_interval,
      base::BindRepeating(&AdBlockSubscriptionServiceManager::OnUpdateTimer,
                          weak_ptr_factory_.GetWeakPtr()),
      base::DoNothing());
}

// static
absl::optional<SubscriptionInfo> AdBlockSubscriptionServiceManager::GetInfo(
    const base::Value::Dict& subscriptions,
    const GURL& sub_url) {
  auto* list_subscription_dict = subscriptions.Find(sub_url.spec());
  if (!list_subscription_dict)
    return absl::nullopt;

  return absl::make_optional<SubscriptionInfo>(
      BuildInfoFromDict(sub_url, list_subscription_dict));
}

void AdBlockSubscriptionServiceManager::LoadSubscriptionServices() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!local_state_)
    return;

  base::AutoLock lock(subscription_services_lock_);
  subscriptions_ = local_state_->GetDictionary(prefs::kAdBlockListSubscriptions)
                       ->GetDict()
                       .Clone();

  for (const auto it : subscriptions_) {
    const std::string key = it.first;
    SubscriptionInfo info;
    const base::Value* list_subscription_dict = subscriptions_.Find(key);
    if (list_subscription_dict && list_subscription_dict->is_dict()) {
      GURL sub_url(key);
      info = BuildInfoFromDict(sub_url, list_subscription_dict);

      auto subscription_service =
          std::unique_ptr<AdBlockEngine, base::OnTaskRunnerDeleter>(
              new AdBlockEngine(), base::OnTaskRunnerDeleter(task_runner_));

      auto subscription_filters_provider =
          std::make_unique<AdBlockSubscriptionFiltersProvider>(
              local_state_,
              GetSubscriptionPath(sub_url).Append(kCustomSubscriptionListText));
      auto observer = std::make_unique<AdBlockService::SourceProviderObserver>(
          subscription_service->AsWeakPtr(),
          subscription_filters_provider.get(), resource_provider_, task_runner_,
          base::BindRepeating(
              &AdBlockSubscriptionServiceManager::OnListMetadata,
              weak_ptr_factory_.GetWeakPtr(), sub_url));

      subscription_services_.insert(
          std::make_pair(sub_url, std::move(subscription_service)));
      subscription_filters_providers_.insert(
          std::make_pair(sub_url, std::move(subscription_filters_provider)));
      subscription_source_observers_.insert(
          std::make_pair(sub_url, std::move(observer)));
    }
  }
}

// Updates preferences to reflect a new state for the specified filter list
// subscription. Creates the entry if it does not yet exist.
void AdBlockSubscriptionServiceManager::UpdateSubscriptionPrefs(
    const GURL& sub_url,
    const SubscriptionInfo& info) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!local_state_)
    return;

  DictionaryPrefUpdate update(local_state_, prefs::kAdBlockListSubscriptions);
  base::Value* subscriptions_dict = update.Get();
  base::Value::Dict subscription_dict;
  subscription_dict.Set("enabled", info.enabled);
  subscription_dict.Set("last_update_attempt",
                        base::TimeToValue(info.last_update_attempt));
  subscription_dict.Set("last_successful_update_attempt",
                        base::TimeToValue(info.last_successful_update_attempt));
  if (info.homepage) {
    subscription_dict.Set("homepage", base::Value(*info.homepage));
  }
  if (info.title) {
    subscription_dict.Set("title", base::Value(*info.title));
  }
  subscriptions_dict->GetDict().Set(sub_url.spec(),
                                    std::move(subscription_dict));

  // TODO(bridiver) - change to pref registrar
  base::AutoLock lock(subscription_services_lock_);
  subscriptions_ = subscriptions_dict->GetDict().Clone();
}

// Updates preferences to remove all state for the specified filter list
// subscription.
void AdBlockSubscriptionServiceManager::ClearSubscriptionPrefs(
    const GURL& sub_url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!local_state_)
    return;

  DictionaryPrefUpdate update(local_state_, prefs::kAdBlockListSubscriptions);
  base::Value* subscriptions_dict = update.Get();
  subscriptions_dict->GetDict().Remove(sub_url.spec());

  // TODO(bridiver) - change to pref registrar
  base::AutoLock lock(subscription_services_lock_);
  subscriptions_ = subscriptions_dict->GetDict().Clone();
}

bool AdBlockSubscriptionServiceManager::Start() {
  return true;
}

void AdBlockSubscriptionServiceManager::ShouldStartRequest(
    const GURL& url,
    blink::mojom::ResourceType resource_type,
    const std::string& tab_host,
    bool aggressive_blocking,
    bool* did_match_rule,
    bool* did_match_exception,
    bool* did_match_important,
    std::string* mock_data_url) {
  base::AutoLock lock(subscription_services_lock_);
  for (const auto& subscription_service : subscription_services_) {
    auto info = GetInfo(subscriptions_, subscription_service.first);
    if (info && info->enabled) {
      subscription_service.second->ShouldStartRequest(
          url, resource_type, tab_host, aggressive_blocking, did_match_rule,
          did_match_exception, did_match_important, mock_data_url);
      if (did_match_important && *did_match_important) {
        return;
      }
    }
  }
}

void AdBlockSubscriptionServiceManager::EnableTag(const std::string& tag,
                                                  bool enabled) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::AutoLock lock(subscription_services_lock_);

  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->EnableTag(tag, enabled);
  }
}

void AdBlockSubscriptionServiceManager::AddResources(
    const std::string& resources) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  base::AutoLock lock(subscription_services_lock_);

  for (const auto& subscription_service : subscription_services_) {
    subscription_service.second->AddResources(resources);
  }
}

absl::optional<base::Value>
AdBlockSubscriptionServiceManager::UrlCosmeticResources(
    const std::string& url) {
  absl::optional<base::Value> first_value = absl::nullopt;

  base::AutoLock lock(subscription_services_lock_);
  for (auto it = subscription_services_.begin();
       it != subscription_services_.end(); it++) {
    auto info = GetInfo(subscriptions_, it->first);
    if (info && info->enabled) {
      absl::optional<base::Value> next_value =
          it->second->UrlCosmeticResources(url);
      if (first_value) {
        if (next_value) {
          MergeResourcesInto(std::move(*next_value), &*first_value, false);
        }
      } else {
        first_value = std::move(next_value);
      }
    }
  }

  return first_value;
}

base::Value AdBlockSubscriptionServiceManager::HiddenClassIdSelectors(
    const std::vector<std::string>& classes,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& exceptions) {
  base::Value first_value(base::Value::Type::LIST);

  base::AutoLock lock(subscription_services_lock_);
  for (auto it = subscription_services_.begin();
       it != subscription_services_.end(); it++) {
    auto info = GetInfo(subscriptions_, it->first);
    if (info && info->enabled) {
      base::Value next_value =
          it->second->HiddenClassIdSelectors(classes, ids, exceptions);
      DCHECK(next_value.is_list());

      for (auto& item : next_value.GetList()) {
        first_value.Append(std::move(item));
      }
    }
  }

  return first_value;
}

void AdBlockSubscriptionServiceManager::OnSubscriptionDownloaded(
    const GURL& sub_url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  auto subscription_filters_provider =
      subscription_filters_providers_.find(sub_url);
  if (subscription_filters_provider == subscription_filters_providers_.end())
    return;

  absl::optional<SubscriptionInfo> info;
  {
    base::AutoLock lock(subscription_services_lock_);
    info = GetInfo(subscriptions_, sub_url);
  }

  if (!info)
    return;

  info->last_update_attempt = base::Time::Now();
  info->last_successful_update_attempt = info->last_update_attempt;
  UpdateSubscriptionPrefs(sub_url, *info);

  auto subscription_source_observer =
      subscription_source_observers_.find(sub_url);
  DCHECK(subscription_source_observer != subscription_source_observers_.end());

  subscription_filters_provider->second->LoadDAT(
      (subscription_source_observer->second).get());

  NotifyObserversOfServiceEvent();
}

void AdBlockSubscriptionServiceManager::OnSubscriptionDownloadFailure(
    const GURL& sub_url) {
  absl::optional<SubscriptionInfo> info;
  {
    base::AutoLock lock(subscription_services_lock_);
    info = GetInfo(subscriptions_, sub_url);
  }

  if (!info)
    return;

  info->last_update_attempt = base::Time::Now();
  UpdateSubscriptionPrefs(sub_url, *info);

  NotifyObserversOfServiceEvent();
}

void AdBlockSubscriptionServiceManager::NotifyObserversOfServiceEvent() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (auto& observer : observers_) {
    observer.OnServiceUpdateEvent();
  }
}

void AdBlockSubscriptionServiceManager::AddObserver(
    AdBlockSubscriptionServiceManagerObserver* observer) {
  observers_.AddObserver(observer);
}

void AdBlockSubscriptionServiceManager::RemoveObserver(
    AdBlockSubscriptionServiceManagerObserver* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace brave_shields
