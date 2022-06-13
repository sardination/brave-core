/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_NTP_BACKGROUND_PREF_H_
#define BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_NTP_BACKGROUND_PREF_H_

#include <string>

#include "base/values.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "url/gurl.h"

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

class PrefService;

// NTPBackgroundPref gives easy access to values for NTP background from prefs.
class NTPBackgroundPref final {
 public:
  static constexpr char kDeprecatedPrefName[] =
      "brave.new_tab_page.custom_background_enabled";
  static constexpr char kPrefName[] = "brave.new_tab_page.background";

  enum class Type {
    kBrave,  // Images that we supply.
    kCustomImage,
    kSolidColor,
    kGradientColor
  };

  explicit NTPBackgroundPref(PrefService* service);
  NTPBackgroundPref(const NTPBackgroundPref& pref) = delete;
  NTPBackgroundPref& operator=(const NTPBackgroundPref& pref) = delete;
  ~NTPBackgroundPref();

  static void RegisterPref(user_prefs::PrefRegistrySyncable* registry);

  // Try to migrate the old pref for custom background into this new pref.
  void MigrateOldPref();

  // Types
  Type GetType() const;
  void SetType(Type type);
  bool IsBraveType() const;
  bool IsCustomImageType() const;
  bool IsSolidColorType() const;
  bool IsGradientColorType() const;

  // Returns true when we should pick one item of selected type every time NTP
  // opens.
  bool ShouldUseRandomValue() const;
  void SetShouldUseRandomValue(bool random);

  void SetSelectedValue(const std::string& value);

  // Return a value to use as NTP background.
  absl::variant<GURL, std::string> GetSelectedValue() const;
  absl::variant<GURL, std::string> GetRandomValue() const;

 private:
  const base::Value::Dict* GetPrefValue() const;

  raw_ptr<PrefService> service_ = nullptr;
};

#endif  // BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_NTP_BACKGROUND_PREF_H_
