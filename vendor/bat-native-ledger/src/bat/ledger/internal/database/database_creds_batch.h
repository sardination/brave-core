/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_DATABASE_DATABASE_CREDS_BATCH_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_DATABASE_DATABASE_CREDS_BATCH_H_

#include <string>
#include <vector>

#include "bat/ledger/internal/database/database_table.h"

namespace ledger {
namespace database {

using GetCredsBatchCallback = std::function<void(type::CredsBatchPtr)>;
using GetCredsBatchListCallback = std::function<void(type::CredsBatchList)>;

class DatabaseCredsBatch: public DatabaseTable {
 public:
  explicit DatabaseCredsBatch(LedgerImpl* ledger);
  ~DatabaseCredsBatch() override;

  void InsertOrUpdate(type::CredsBatchPtr creds,
                      ledger::LegacyResultCallback callback);

  void GetRecordByTrigger(
      const std::string& trigger_id,
      const type::CredsBatchType trigger_type,
      GetCredsBatchCallback callback);

  void SaveSignedCreds(type::CredsBatchPtr creds,
                       ledger::LegacyResultCallback callback);

  void GetAllRecords(GetCredsBatchListCallback callback);

  void UpdateStatus(const std::string& trigger_id,
                    type::CredsBatchType trigger_type,
                    type::CredsBatchStatus status,
                    ledger::LegacyResultCallback callback);

  void UpdateRecordsStatus(const std::vector<std::string>& trigger_ids,
                           type::CredsBatchType trigger_type,
                           type::CredsBatchStatus status,
                           ledger::LegacyResultCallback callback);

  void GetRecordsByTriggers(
      const std::vector<std::string>& trigger_ids,
      GetCredsBatchListCallback callback);

 private:
  void OnGetRecordByTrigger(
      type::DBCommandResponsePtr response,
      GetCredsBatchCallback callback);

  void OnGetRecords(
      type::DBCommandResponsePtr response,
      GetCredsBatchListCallback callback);
};

}  // namespace database
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_DATABASE_DATABASE_CREDS_BATCH_H_
