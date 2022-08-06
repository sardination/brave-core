/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/frame/brave_browser_view_layout.h"

#include "brave/browser/ui/views/tabs/brave_vertical_tab_utils.h"

BraveBrowserViewLayout::~BraveBrowserViewLayout() = default;

int BraveBrowserViewLayout::LayoutTabStripRegion(int top) {
  if (tabs::ShouldShowVerticalTabs())
    return top;

  return BrowserViewLayout::LayoutTabStripRegion(top);
}
