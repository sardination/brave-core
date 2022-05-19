/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/page/create_window.h"

#include "brave/third_party/blink/renderer/core/brave_session_cache.h"
#include "third_party/blink/public/web/web_window_features.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/screen.h"

// Because we are spoofing screenX and screenY, we need to offset the position
// when a page script opens a new window in screen coordinates. And because
// we are spoofing screen width and height, we should artificially limit the
// window size to that width and height as well, so that window.open can't
// be used to probe the screen size.

namespace blink {

WebWindowFeatures MaybeFarbleWindowFeatures(const String& feature_string,
                                            LocalDOMWindow* dom_window) {
  WebWindowFeatures window_features =
      GetWindowFeaturesFromString_ChromiumImpl(feature_string, dom_window);
  ExecutionContext* context = dom_window->GetExecutionContext();
  if (brave::BlockScreenFingerprinting(context)) {
    if (window_features.x_set) {
      window_features.x += dom_window->screenX_ChromiumImpl();
    }
    if (window_features.y_set) {
      window_features.y += dom_window->screenY_ChromiumImpl();
    }
    if (window_features.width_set) {
      int maxWidth = dom_window->screen()->width();
      if (window_features.width > maxWidth) {
        window_features.width = maxWidth;
      }
    }
    if (window_features.height_set) {
      int maxHeight = dom_window->screen()->height();
      if (window_features.height > maxHeight) {
        window_features.height = maxHeight;
      }
    }
  }
  return window_features;
}

}  // namespace blink

#define GetWindowFeaturesFromString                               \
  GetWindowFeaturesFromString(const String& feature_string,       \
                              LocalDOMWindow* dom_window) {       \
    return MaybeFarbleWindowFeatures(feature_string, dom_window); \
  }                                                               \
  WebWindowFeatures GetWindowFeaturesFromString_ChromiumImpl

#include "src/third_party/blink/renderer/core/page/create_window.cc"

#undef GetWindowFeaturesFromString
