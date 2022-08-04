/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>

#include "base/files/scoped_temp_dir.h"

#include "chrome/browser/prefs/browser_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/chrome_sync_client.h"
#include "chrome/browser/sync/sync_service_factory.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "components/search_engines/search_engines_pref_names.h"
#include "components/sync/driver/test_sync_service.h"
#include "components/sync_preferences/pref_service_mock_factory.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

std::unique_ptr<Profile> CreateProfile(const base::FilePath& path) {
  SyncServiceFactory::GetInstance();

  sync_preferences::PrefServiceMockFactory factory;
  auto registry = base::MakeRefCounted<user_prefs::PrefRegistrySyncable>();
  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs(
      factory.CreateSyncable(registry.get()));
  RegisterUserProfilePrefs(registry.get());

  TestingProfile::Builder profile_builder;
  profile_builder.SetPrefService(std::move(prefs));
  profile_builder.SetPath(path);
  return profile_builder.Build();
}

}  // namespace

namespace browser_sync {

class BraveSyncClientTest : public testing::Test {
 public:
  BraveSyncClientTest() {}
  ~BraveSyncClientTest() override {}

 protected:
  void SetUp() override {
    EXPECT_TRUE(temp_dir_.CreateUniqueTempDir());
    profile_ = CreateProfile(temp_dir_.GetPath());
    EXPECT_TRUE(profile_.get() != NULL);
  }

  Profile* profile() { return profile_.get(); }

 private:
  // Need this as a very first member to run tests in UI thread
  // When this is set, class should not install any other MessageLoops, like
  // base::test::ScopedTaskEnvironment
  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<Profile> profile_;
  base::ScopedTempDir temp_dir_;
};

TEST_F(BraveSyncClientTest, CreateDataTypeControllersSearchEngines) {
  auto sync_client =
      std::make_unique<browser_sync::ChromeSyncClient>(profile());

  syncer::TestSyncService service;
  const syncer::DataTypeController::TypeVector controllers =
      sync_client->CreateDataTypeControllers(&service);

  auto search_engines_controller_it = std::find_if(
      controllers.begin(), controllers.end(),
      [](const std::unique_ptr<syncer::DataTypeController>& controller) {
        return controller->type() == syncer::SEARCH_ENGINES;
      });

  EXPECT_NE(search_engines_controller_it, controllers.end());
}

TEST_F(BraveSyncClientTest, PrefSyncedDefaultSearchProviderGUIDIsSyncable) {
  // This test supposed to be near
  // components/search_engines/template_url_service.cc
  // But we have it here because we have profile here and both these tests are
  // related by the final purpose
  const PrefService::Preference* pref = profile()->GetPrefs()->FindPreference(
      prefs::kSyncedDefaultSearchProviderGUID);
  EXPECT_TRUE(pref->registration_flags() &
              user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
}

}  // namespace browser_sync
