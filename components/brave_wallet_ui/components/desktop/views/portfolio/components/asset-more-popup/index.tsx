// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

import { getLocale } from '../../../../../../../common/locale'

// Styled Components
import {
  StyledWrapper,
  PopupButton,
  PopupButtonText,
  HelpCenterIcon
} from './style'

interface Props {
  onClickTokenDetails: () => void
}

export const AssetMorePopup = (props: Props) => {
  const { onClickTokenDetails } = props

  return (
    <StyledWrapper>
      <PopupButton onClick={onClickTokenDetails}>
        <HelpCenterIcon />
        <PopupButtonText>{getLocale('braveWalletPortfolioTokenDetailsMenuLabel')}</PopupButtonText>
      </PopupButton>
    </StyledWrapper>
  )
}
