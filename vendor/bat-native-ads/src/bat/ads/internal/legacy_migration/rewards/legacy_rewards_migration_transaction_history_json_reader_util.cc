/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/legacy_migration/rewards/legacy_rewards_migration_transaction_history_json_reader_util.h"

#include <string>

#include "base/guid.h"
#include "base/strings/string_number_conversions.h"
#include "bat/ads/confirmation_type.h"

namespace ads {
namespace rewards {
namespace JSONReader {

namespace {

constexpr char kTransactionHistoryKey[] = "transaction_history";
constexpr char kTransactionListKey[] = "transactions";
constexpr char kCreatedAtKey[] = "timestamp_in_seconds";
constexpr char kValueKey[] = "estimated_redemption_value";
constexpr char kConfirmationTypeKey[] = "confirmation_type";

absl::optional<TransactionInfo> ParseTransaction(
    const base::Value::Dict& value) {
  TransactionInfo transaction;

  // Id
  transaction.id = base::GUID::GenerateRandomV4().AsLowercaseString();

  // Created at
  const std::string* created_at = value.FindString(kCreatedAtKey);
  if (!created_at) {
    return absl::nullopt;
  }

  double created_at_as_double = 0.0;
  if (!base::StringToDouble(*created_at, &created_at_as_double)) {
    return absl::nullopt;
  }
  transaction.created_at = base::Time::FromDoubleT(created_at_as_double);

  // Value
  const absl::optional<double> value_optional = value.FindDouble(kValueKey);
  if (!value_optional) {
    return absl::nullopt;
  }
  transaction.value = value_optional.value();

  // Confirmation type
  const std::string* confirmation_type = value.FindString(kConfirmationTypeKey);
  if (!confirmation_type) {
    return absl::nullopt;
  }
  transaction.confirmation_type = ConfirmationType(*confirmation_type);

  return transaction;
}

absl::optional<TransactionList> GetTransactionsFromList(
    const base::Value::List& value) {
  TransactionList transactions;

  for (const auto& transaction_value : value) {
    if (!transaction_value.is_dict()) {
      return absl::nullopt;
    }

    const absl::optional<TransactionInfo>& transaction_optional =
        ParseTransaction(transaction_value.GetDict());
    if (!transaction_optional) {
      return absl::nullopt;
    }
    TransactionInfo transaction = transaction_optional.value();

    transactions.push_back(transaction);
  }

  return transactions;
}

}  // namespace

absl::optional<TransactionList> ParseTransactionHistory(
    const base::Value::Dict& value) {
  const base::Value::Dict* transaction_history_value =
      value.FindDict(kTransactionHistoryKey);
  if (!transaction_history_value) {
    return absl::nullopt;
  }

  const base::Value::List* transaction_value =
      transaction_history_value->FindList(kTransactionListKey);
  if (!transaction_value) {
    return absl::nullopt;
  }

  const absl::optional<TransactionList>& transactions_optional =
      GetTransactionsFromList(*transaction_value);
  if (!transactions_optional) {
    return absl::nullopt;
  }

  return transactions_optional.value();
}

}  // namespace JSONReader
}  // namespace rewards
}  // namespace ads
