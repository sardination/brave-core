/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_IOS_BROWSER_API_BRAVE_WALLET_BRAVE_WALLET_PROVIDER_DELEGATE_IOS_PRIVATE_H_
#define BRAVE_IOS_BROWSER_API_BRAVE_WALLET_BRAVE_WALLET_PROVIDER_DELEGATE_IOS_PRIVATE_H_

#include <string>
#include <vector>

#include "brave/components/brave_wallet/browser/brave_wallet_provider_delegate.h"

@protocol BraveWalletProviderDelegate;

namespace brave_wallet {

class BraveWalletProviderDelegateBridge
    : public brave_wallet::BraveWalletProviderDelegate {
 public:
  explicit BraveWalletProviderDelegateBridge(
      id<BraveWalletProviderDelegate> bridge)
      : bridge_(bridge) {}

 private:
  __weak id<BraveWalletProviderDelegate> bridge_;

  bool IsTabVisible() override;
  void ShowPanel() override;
  void WalletInteractionDetected() override;
  url::Origin GetOrigin() const override;
  void ShowWalletOnboarding() override;
  void ShowAccountCreation(mojom::CoinType type) override;
  void RequestPermissions(mojom::CoinType type,
                          const std::vector<std::string>& accounts,
                          RequestPermissionsCallback) override;
  void IsAccountAllowed(mojom::CoinType type,
                        const std::string& account,
                        IsAccountAllowedCallback callback) override;
  void GetAllowedAccounts(mojom::CoinType type,
                          const std::vector<std::string>& accounts,
                          GetAllowedAccountsCallback callback) override;
  bool IsPermissionDenied(mojom::CoinType type) override;
};

}  // namespace brave_wallet

#endif  // BRAVE_IOS_BROWSER_API_BRAVE_WALLET_BRAVE_WALLET_PROVIDER_DELEGATE_IOS_PRIVATE_H_
