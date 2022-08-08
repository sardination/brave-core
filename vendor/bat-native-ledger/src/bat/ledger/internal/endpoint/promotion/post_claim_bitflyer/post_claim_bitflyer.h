/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_PROMOTION_POST_CLAIM_BITFLYER_POST_CLAIM_BITFLYER_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_PROMOTION_POST_CLAIM_BITFLYER_POST_CLAIM_BITFLYER_H_

#include <string>

#include "bat/ledger/internal/endpoint/endpoint.h"

// POST /v3/wallet/bitflyer/{payment_id}/claim
//
// Request body:
// {
//   "linkingInfo": "83b3b77b-e7c3-455b-adda-e476fa0656d2"
// }
//
// Success code:
// HTTP_OK (200)
//
// Error codes:
// HTTP_BAD_REQUEST (400)
// HTTP_NOT_FOUND (404)
// HTTP_CONFLICT (409)
// HTTP_INTERNAL_SERVER_ERROR (500)
//
// Response body:
// {Empty}

namespace ledger {
class LedgerImpl;

namespace endpoint {
namespace promotion {

class ClaimBitflyer : public Endpoint<type::UrlMethod::POST, ClaimBitflyer> {
  friend Endpoint<type::UrlMethod::POST, ClaimBitflyer>;

 public:
  using Callback = base::OnceCallback<void(type::Result)>;

  explicit ClaimBitflyer(LedgerImpl* ledger, const std::string& linking_info);
  ~ClaimBitflyer() override;

 private:
  std::string Url() override;
  std::string Content() override;
  std::vector<std::string> Headers() override;

  std::string GeneratePayload(const std::string& linking_info);

  static type::Result ProcessResponse(const type::UrlResponse& response);

  static type::Result ParseBody(const std::string& body);

  static void OnLoadURL(Callback callback,
                        const type::UrlResponse& response);

  std::string linking_info_;
};

}  // namespace promotion
}  // namespace endpoint
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_PROMOTION_POST_CLAIM_BITFLYER_POST_CLAIM_BITFLYER_H_
