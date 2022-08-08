/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoint/endpoint_base.h"
#include "bat/ledger/internal/ledger_impl.h"

namespace ledger::endpoint {
EndpointBase::EndpointBase(LedgerImpl* ledger)
    : ledger_((DCHECK(ledger), ledger)) {}

void EndpointBase::LoadURL(type::UrlRequestPtr request,
                           client::LoadURLCallback callback) {
  ledger_->LoadURL(std::move(request), std::move(callback));
}
};  // namespace ledger::endpoint
