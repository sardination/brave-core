/* Copyright 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/common/importer/chrome_importer_utils.h"

#include <memory>
#include <utility>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/values.h"
#include "brave/common/importer/importer_constants.h"
#include "brave/common/importer/scoped_copy_file.h"
#include "chrome/common/importer/importer_data_types.h"
#include "components/webdata/common/webdata_constants.h"
#include "sql/database.h"
#include "sql/statement.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
using extensions::Extension;
using extensions::Manifest;
#endif

namespace {

#if BUILDFLAG(ENABLE_EXTENSIONS)
bool HasImportableExtensions(const base::FilePath& secured_preference_path) {
  if (!base::PathExists(secured_preference_path))
    return false;

  std::string secured_preference_content;
  base::ReadFileToString(secured_preference_path, &secured_preference_content);
  absl::optional<base::Value> secured_preference =
      base::JSONReader::Read(secured_preference_content);
  if (auto* extensions = secured_preference->FindPath(
          kChromeExtensionsListPath)) {
    auto extensions_list =
        GetImportableListFromChromeExtensionsList(*extensions);
    return !extensions_list.empty();
  }
  return false;
}
#endif

bool HasPaymentMethods(const base::FilePath& payments_path) {
  if (!base::PathExists(payments_path))
    return false;

  ScopedCopyFile copy_payments_file(payments_path);
  if (!copy_payments_file.copy_success())
    return false;

  sql::Database db;
  if (!db.Open(copy_payments_file.copied_file_path())) {
    return false;
  }

  constexpr char query[] = "SELECT name_on_card FROM credit_cards;";
  sql::Statement s(db.GetUniqueStatement(query));
  // Will return false if there is no payment info.
  return s.Step();
}

}  // namespace

base::Value::List GetChromeSourceProfiles(
    const base::FilePath& local_state_path) {
  base::Value::List profiles;
  if (base::PathExists(local_state_path)) {
    std::string local_state_content;
    base::ReadFileToString(local_state_path, &local_state_content);
    absl::optional<base::Value> local_state =
        base::JSONReader::Read(local_state_content);
    if (!local_state)
      return profiles;

    const auto* local_state_dict = local_state->GetIfDict();
    if (!local_state_dict)
      return profiles;

    const auto* profile_dict = local_state_dict->FindDict("profile");
    if (profile_dict) {
      const auto* info_cache = profile_dict->FindDict("info_cache");
      if (info_cache) {
        for (const auto value : *info_cache) {
          const auto* profile = value.second.GetIfDict();
          if (!profile)
            continue;

          auto* name = profile->FindString("name");
          DCHECK(name);
          base::Value::Dict entry;
          entry.Set("id", value.first);
          entry.Set("name", *name);
          profiles.Append(std::move(entry));
        }
      }
    }
  }
  if (profiles.empty()) {
    base::Value::Dict entry;
    entry.Set("id", "");
    entry.Set("name", "Default");
    profiles.Append(std::move(entry));
  }
  return profiles;
}

bool ChromeImporterCanImport(const base::FilePath& profile,
                             uint16_t* services_supported) {
  DCHECK(services_supported);
  *services_supported = importer::NONE;

  base::FilePath bookmarks =
    profile.Append(base::FilePath::StringType(FILE_PATH_LITERAL("Bookmarks")));
  base::FilePath history =
    profile.Append(base::FilePath::StringType(FILE_PATH_LITERAL("History")));
  base::FilePath passwords =
    profile.Append(base::FilePath::StringType(FILE_PATH_LITERAL("Login Data")));
  base::FilePath secured_preference =
      profile.AppendASCII(kChromeExtensionsPreferencesFile);
  if (base::PathExists(bookmarks))
    *services_supported |= importer::FAVORITES;
  if (base::PathExists(history))
    *services_supported |= importer::HISTORY;
  if (base::PathExists(passwords))
    *services_supported |= importer::PASSWORDS;
  if (HasPaymentMethods(profile.Append(kWebDataFilename)))
    *services_supported |= importer::PAYMENTS;
#if BUILDFLAG(ENABLE_EXTENSIONS)
  if (HasImportableExtensions(secured_preference))
    *services_supported |= importer::EXTENSIONS;
#endif

  return *services_supported != importer::NONE;
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
std::vector<std::string> GetImportableListFromChromeExtensionsList(
    const base::Value& extensions_list) {
  DCHECK(extensions_list.is_dict());

  std::vector<std::string> extensions;
  for (const auto item : extensions_list.DictItems()) {
    // Only import if type is extension, it's came from webstore and it's not
    // installed by default.
    if (item.second.FindBoolKey("was_installed_by_default").value_or(true))
        continue;

    if (!item.second.FindBoolKey("from_webstore").value_or(false))
      continue;

    if (auto* manifest_value = item.second.FindDictKey("manifest")) {
      if (!manifest_value->is_dict())
        continue;

      const auto& manifest = base::Value::AsDictionaryValue(*manifest_value);
      if (Manifest::GetTypeFromManifestValue(manifest) ==
          Manifest::TYPE_EXTENSION) {
        extensions.push_back(item.first);
      }
    }
  }

  return extensions;
}
#endif
