/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/uphold/uphold_user.h"

#include <algorithm>
#include <utility>

#include "base/json/json_reader.h"
#include "bat/ledger/global_constants.h"
#include "bat/ledger/internal/endpoint/uphold/uphold_server.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/uphold/uphold_util.h"

namespace ledger {
namespace uphold {

User::User() : name(""), member_id(""), bat_not_allowed(true) {}

User::~User() = default;

UpholdUser::UpholdUser(LedgerImpl* ledger) :
    ledger_(ledger),
    uphold_server_(std::make_unique<endpoint::UpholdServer>(ledger)) {
}

UpholdUser::~UpholdUser() = default;

void UpholdUser::Get(GetUserCallback callback) {
  auto wallet = ledger_->uphold()->GetWallet();

  if (!wallet) {
    User user;
    BLOG(0, "Wallet is null");
    std::move(callback).Run(type::Result::LEDGER_ERROR, user);
    return;
  }

  auto url_callback = base::BindOnce(&UpholdUser::OnGet, base::Unretained(this),
                                     std::move(callback));

  uphold_server_->get_me()->Request(wallet->token, std::move(url_callback));
}

void UpholdUser::OnGet(GetUserCallback callback,
                       type::Result result,
                       const User& user) {
  if (result == type::Result::EXPIRED_TOKEN) {
    BLOG(0, "Expired token");
    std::move(callback).Run(type::Result::EXPIRED_TOKEN, user);
    return;
  }

  if (result != type::Result::LEDGER_OK) {
    BLOG(0, "Couldn't get user");
    std::move(callback).Run(type::Result::LEDGER_ERROR, user);
    return;
  }

  std::move(callback).Run(type::Result::LEDGER_OK, user);
}

}  // namespace uphold
}  // namespace ledger
