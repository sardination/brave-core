/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ntp_background_images/ntp_background_pref.h"

#include <utility>

#include "base/notreached.h"
#include "brave/components/constants/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace {

constexpr char kTypeKey[] = "type";
constexpr char kRandomKey[] = "random";
constexpr char kSelectedValueKey[] = "selected_value";
>>>>>>> 0341a77c6b (Introduce NTPBackgroundPref)

const char* TypeToString(NTPBackgroundPref::Type type) {
  switch (type) {
    case NTPBackgroundPref::Type::kBrave:
      return "brave";
    case NTPBackgroundPref::Type::kCustomImage:
      return "custom_image";
    case NTPBackgroundPref::Type::kSolidColor:
      return "solid_color";
    case NTPBackgroundPref::Type::kGradientColor:
      return "gradient_color";
  }
}

NTPBackgroundPref::Type StringToType(const std::string& type_string) {
  if (type_string == "brave")
    return NTPBackgroundPref::Type::kBrave;
  if (type_string == "custom_image")
    return NTPBackgroundPref::Type::kCustomImage;
  if (type_string == "solid_color")
    return NTPBackgroundPref::Type::kSolidColor;
  if (type_string == "gradient_color")
    return NTPBackgroundPref::Type::kGradientColor;

  NOTREACHED();
  return NTPBackgroundPref::Type::kBrave;
}

}  // namespace

NTPBackgroundPref::NTPBackgroundPref(PrefService* service)
    : service_(service) {}

NTPBackgroundPref::~NTPBackgroundPref() = default;

// static
void NTPBackgroundPref::RegisterPref(
    user_prefs::PrefRegistrySyncable* registry) {
  base::Value::Dict dict;
  dict.Set(kTypeKey, TypeToString(Type::kBrave));
  dict.Set(kRandomKey, false);
  dict.Set(kSelectedValueKey, "");
  registry->RegisterDictionaryPref(kPrefName, base::Value(std::move(dict)));
}

void NTPBackgroundPref::MigrateOldPref() {
  if (!service_->HasPrefPath(kDeprecatedPrefName))
    return;

  if (service_->GetBoolean(kDeprecatedPrefName))
    SetType(Type::kCustomImage);

  service_->ClearPref(kDeprecatedPrefName);
}

NTPBackgroundPref::Type NTPBackgroundPref::GetType() const {
  const auto* value = GetPrefValue();
  const auto* type_string = value->FindString(kTypeKey);
  return StringToType(*type_string);
}

void NTPBackgroundPref::SetType(Type type) {
  if (type == GetType())
    return;

  auto new_value = GetPrefValue()->Clone();
  new_value.Set(kTypeKey, TypeToString(type));
  service_->SetDict(kPrefName, std::move(new_value));
}

bool NTPBackgroundPref::IsBraveType() const {
  return GetType() == Type::kBrave;
}

bool NTPBackgroundPref::IsCustomImageType() const {
  return GetType() == Type::kCustomImage;
}

bool NTPBackgroundPref::IsSolidColorType() const {
  return GetType() == Type::kSolidColor;
}

bool NTPBackgroundPref::IsGradientColorType() const {
  return GetType() == Type::kGradientColor;
}

bool NTPBackgroundPref::ShouldUseRandomValue() const {
  const auto* value = GetPrefValue();
  auto optional_bool = value->FindBool(kRandomKey);
  return optional_bool.value_or(false);
}

void NTPBackgroundPref::SetShouldUseRandomValue(bool random) {
  NOTIMPLEMENTED();
}

void NTPBackgroundPref::SetSelectedValue(const std::string& value) {
  auto new_value = GetPrefValue()->Clone();
  new_value.Set(kSelectedValueKey, value);
  service_->SetDict(kPrefName, std::move(new_value));
}

absl::variant<GURL, std::string> NTPBackgroundPref::GetSelectedValue() const {
  const auto* value = GetPrefValue();
  const auto* selected_value = value->FindString(kSelectedValueKey);
  DCHECK(selected_value);

  if (auto type = GetType();
      type == Type::kSolidColor || type == Type::kGradientColor)
    return *selected_value;

  return GURL(*selected_value);
}

absl::variant<GURL, std::string> NTPBackgroundPref::GetRandomValue() const {
  NOTIMPLEMENTED();
  return std::string{};
}

const base::Value::Dict* NTPBackgroundPref::GetPrefValue() const {
  const auto* value = service_->GetDictionary(kPrefName);
  DCHECK(value && value->is_dict());
  return value->GetIfDict();
}
