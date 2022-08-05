// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

// types
import { BraveWallet } from '../../../../../../constants/types'

// utils
import { getLocale } from '../../../../../../../common/locale'

// components
import PopupModal from '../../../../../desktop/popup-modals'
import {
  StyledWrapper,
  TokenBalanceRow,
  CryptoBalance,
  DetailColumn,
  TokenDetailLabel,
  TokenDetailValue,
  FiatBalance,
  ContractAddress,
  HideTokenButton,
  modalWidth
} from './token-details-modal-styles'
import withPlaceholderIcon from '../../../../../shared/create-placeholder-icon'
import { AssetIcon } from '../../style'
import CopyTooltip from '../../../../../shared/copy-tooltip/copy-tooltip'

interface Props {
  selectedAsset: BraveWallet.BlockchainToken
  selectedAssetNetwork: BraveWallet.NetworkInfo
  onClose: () => void
}

const AssetIconWithPlaceholder = withPlaceholderIcon(AssetIcon, { size: 'big', marginLeft: 0, marginRight: 12 })

export const TokenDetailsModal = (props: Props) => {
  const { selectedAsset, selectedAssetNetwork, onClose } = props

  return (
    <PopupModal
      title={getLocale('braveWalletPortfolioTokenDetailsMenuLabel')}
      onClose={onClose}
    >
      <StyledWrapper>
        <TokenBalanceRow>
          <CryptoBalance>100</CryptoBalance>
          <AssetIconWithPlaceholder asset={selectedAsset} network={selectedAssetNetwork} />
        </TokenBalanceRow>
        <FiatBalance>$1000.00</FiatBalance>
        <DetailColumn>
          <TokenDetailLabel>Token Contract Address</TokenDetailLabel>
          <CopyTooltip text={selectedAsset.contractAddress}>
            <ContractAddress>{selectedAsset.contractAddress}</ContractAddress>
          </CopyTooltip>
        </DetailColumn>
        <DetailColumn>
          <TokenDetailLabel>Token Decimal</TokenDetailLabel>
          <TokenDetailValue>{selectedAsset.decimals}</TokenDetailValue>
        </DetailColumn>
        <DetailColumn>
          <TokenDetailLabel>Network</TokenDetailLabel>
          <TokenDetailValue>{selectedAssetNetwork.chainName}</TokenDetailValue>
        </DetailColumn>
        <HideTokenButton>Hide token</HideTokenButton>
      </StyledWrapper>
    </PopupModal>
  )
}
