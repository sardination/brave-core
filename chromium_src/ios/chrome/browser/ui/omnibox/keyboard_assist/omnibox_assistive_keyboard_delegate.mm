// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/omnibox/keyboard_assist/omnibox_assistive_keyboard_delegate.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@implementation OmniboxAssistiveKeyboardDelegateImpl

@synthesize dispatcher = _dispatcher;
@synthesize omniboxTextField = _omniboxTextField;
@synthesize voiceSearchButtonGuide = _voiceSearchButtonGuide;

#pragma mark - Public

- (void)keyboardAccessoryVoiceSearchTouchUpInside:(UIView*)view {
  
}

- (void)keyboardAccessoryCameraSearchTouchUp {

}

- (void)keyPressed:(NSString*)title {

}

#pragma mark - Private

// Insert 'com' without the period if cursor is directly after a period.
- (NSString*)updateTextForDotCom:(NSString*)text {
  return text;
}

@end
