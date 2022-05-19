/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_CSS_MEDIA_VALUES_CC_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_CSS_MEDIA_VALUES_CC_

#include "third_party/blink/renderer/core/css/media_values.h"

#include "brave/third_party/blink/renderer/core/brave_session_cache.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/screen.h"

#include "brave/third_party/blink/renderer/core/brave_session_cache.h"

#define CalculateDeviceWidth                                                   \
  CalculateDeviceWidth(LocalFrame* frame) {                                    \
    return MaybeFarbleScreenInteger(frame->DomWindow()->GetExecutionContext(), \
                                    brave::FarbleKey::WINDOW_INNERWIDTH,       \
                                    frame->DomWindow()->innerWidth(), 0, 8,    \
                                    CalculateDeviceWidth_ChromiumImpl(frame)); \
  }                                                                            \
  int MediaValues::CalculateDeviceWidth_ChromiumImpl

#define CalculateDeviceHeight                       \
  CalculateDeviceHeight(LocalFrame* frame) {        \
    return MaybeFarbleScreenInteger(                \
        frame->DomWindow()->GetExecutionContext(),  \
        brave::FarbleKey::WINDOW_INNERHEIGHT,       \
        frame->DomWindow()->innerHeight(), 0, 8,    \
        CalculateDeviceHeight_ChromiumImpl(frame)); \
  }                                                 \
  int MediaValues::CalculateDeviceHeight_ChromiumImpl

#include "src/third_party/blink/renderer/core/css/media_values.cc"

#undef CalculateDeviceWidth
#undef CalculateDeviceHeight

// BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_CSS_MEDIA_VALUES_CC_
#endif
