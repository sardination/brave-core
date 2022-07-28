/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LayoutContext, getPreferredLayout } from '../lib/layout_context'
import { useActions, useRewardsData } from '../lib/redux_hooks'
import { Settings } from './settings'

export function App () {
  const [layoutKind, setLayoutKind] = React.useState(getPreferredLayout())
  const actions = useActions()
  const rewardsData = useRewardsData((data) => ({
    initializing: data.initializing,
    adsData: data.adsData,
    enabledAdsMigrated: data.enabledAdsMigrated
  }))

  React.useEffect(() => {
    actions.isInitialized()

    if (!rewardsData.enabledAdsMigrated) {
      const { adsEnabled, adsIsSupported } = rewardsData.adsData
      if (adsIsSupported) {
        actions.onAdsSettingSave('adsEnabledMigrated', adsEnabled)
      }
    }
  }, [])

  React.useEffect(() => {
    const onResize = () => { setLayoutKind(getPreferredLayout()) }
    window.addEventListener('resize', onResize)
    return () => { window.removeEventListener('resize', onResize) }
  }, [])

  return (
    <LayoutContext.Provider value={layoutKind}>
      <div id='rewardsPage'>
        {!rewardsData.initializing && <Settings />}
      </div>
    </LayoutContext.Provider>
  )
}
