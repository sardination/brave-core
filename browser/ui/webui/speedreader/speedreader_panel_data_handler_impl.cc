// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/speedreader/speedreader_panel_data_handler_impl.h"

#include <utility>

#include "brave/browser/speedreader/speedreader_tab_helper.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"

SpeedreaderPanelDataHandlerImpl::SpeedreaderPanelDataHandlerImpl(
    mojo::PendingReceiver<speedreader::mojom::PanelDataHandler> receiver,
    TabStripModel* tab_strip_model)
    : receiver_(this, std::move(receiver)) {
  DCHECK(tab_strip_model);

  auto* web_contents = tab_strip_model->GetActiveWebContents();
  if (!web_contents)
    return;
  speedreader_tab_helper_ =
      speedreader::SpeedreaderTabHelper::FromWebContents(web_contents);
}

SpeedreaderPanelDataHandlerImpl::~SpeedreaderPanelDataHandlerImpl() = default;

void SpeedreaderPanelDataHandlerImpl::GetTheme(GetThemeCallback callback) {
  DCHECK(speedreader_tab_helper_);
  std::move(callback).Run(speedreader_tab_helper_->GetTheme());
}

void SpeedreaderPanelDataHandlerImpl::SetTheme(Theme theme) {
  DCHECK(speedreader_tab_helper_);
  speedreader_tab_helper_->SetTheme(theme);
}
