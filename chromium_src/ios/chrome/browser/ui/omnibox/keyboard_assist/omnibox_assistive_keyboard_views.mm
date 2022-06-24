// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/omnibox/keyboard_assist/omnibox_assistive_keyboard_views.h"

#include "base/check.h"
#import "ios/chrome/browser/ui/omnibox/keyboard_assist/omnibox_assistive_keyboard_delegate.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

void ConfigureAssistiveKeyboardViews(
    UITextField* textField,
    NSString* dotComTLD,
    id<OmniboxAssistiveKeyboardDelegate> delegate) {
  DCHECK(dotComTLD);
}
