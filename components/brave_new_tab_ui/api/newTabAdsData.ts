// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as Cr from 'chrome://resources/js/cr.m'

export type NewTabAdsData = {
  needsBrowserUpgradeToServeAds: boolean
}

type NewTabAdsDataUpdatedHandler = (data: NewTabAdsData) => void

export function getNewTabAdsData (): Promise<NewTabAdsData> {
  return Cr.sendWithPromise('getNewTabAdsData')
}

export function addChangeListener (listener: NewTabAdsDataUpdatedHandler): void {
  Cr.addWebUIListener('new-tab-ads-data-updated', listener)
}
