// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/settings_navigation_controller.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

NSString* const kSettingsDoneButtonId = @"kSettingsDoneButtonId";

@implementation SettingsNavigationController

#pragma mark - SettingsNavigationController methods.

+ (instancetype)
    mainSettingsControllerForBrowser:(Browser*)browser
                            delegate:(id<SettingsNavigationControllerDelegate>)
                                         delegate {
  DCHECK(browser);
  return nil;
}

+ (instancetype)
    accountsControllerForBrowser:(Browser*)browser
                        delegate:
                            (id<SettingsNavigationControllerDelegate>)delegate {
  return nil;
}

+ (instancetype)
    googleServicesControllerForBrowser:(Browser*)browser
                              delegate:
                                  (id<SettingsNavigationControllerDelegate>)
                                      delegate {
  return nil;
}

+ (instancetype)
    syncSettingsControllerForBrowser:(Browser*)browser
                            delegate:(id<SettingsNavigationControllerDelegate>)
                                         delegate {
  return nil;
}

+ (instancetype)
    safetyCheckControllerForBrowser:(Browser*)browser
                           delegate:(id<SettingsNavigationControllerDelegate>)
                                        delegate {
  return nil;
}

+ (instancetype)
    safeBrowsingControllerForBrowser:(Browser*)browser
                            delegate:(id<SettingsNavigationControllerDelegate>)
                                         delegate {
  return nil;
}

+ (instancetype)
    syncPassphraseControllerForBrowser:(Browser*)browser
                              delegate:
                                  (id<SettingsNavigationControllerDelegate>)
                                      delegate {
  return nil;
}

+ (instancetype)
    savePasswordsControllerForBrowser:(Browser*)browser
                             delegate:(id<SettingsNavigationControllerDelegate>)
                                          delegate
      startPasswordCheckAutomatically:(BOOL)startCheck
                     showCancelButton:(BOOL)showCancelButton {
  return nil;
}

+ (instancetype)
    userFeedbackControllerForBrowser:(Browser*)browser
                            delegate:(id<SettingsNavigationControllerDelegate>)
                                         delegate
                  feedbackDataSource:(id<UserFeedbackDataSource>)dataSource
                              sender:(UserFeedbackSender)sender
                             handler:(id<ApplicationCommands>)handler {
  return nil;
}

+ (instancetype)
    importDataControllerForBrowser:(Browser*)browser
                          delegate:
                              (id<SettingsNavigationControllerDelegate>)delegate
                importDataDelegate:
                    (id<ImportDataControllerDelegate>)importDataDelegate
                         fromEmail:(NSString*)fromEmail
                           toEmail:(NSString*)toEmail {
  return nil;
}

+ (instancetype)
    autofillProfileControllerForBrowser:(Browser*)browser
                               delegate:
                                   (id<SettingsNavigationControllerDelegate>)
                                       delegate {
  return nil;
}

+ (instancetype)
    autofillCreditCardControllerForBrowser:(Browser*)browser
                                  delegate:
                                      (id<SettingsNavigationControllerDelegate>)
                                          delegate {
  return nil;
}

+ (instancetype)
    defaultBrowserControllerForBrowser:(Browser*)browser
                              delegate:
                                  (id<SettingsNavigationControllerDelegate>)
                                      delegate {
  return nil;
}

+ (instancetype)
    clearBrowsingDataControllerForBrowser:(Browser*)browser
                                 delegate:
                                     (id<SettingsNavigationControllerDelegate>)
                                         delegate {
  return nil;
}

#pragma mark - Lifecycle

- (instancetype)initWithRootViewController:(UIViewController*)rootViewController
                                   browser:(Browser*)browser
                                  delegate:
                                      (id<SettingsNavigationControllerDelegate>)
                                          delegate {
  return (self = [super initWithRootViewController:rootViewController]);
}

#pragma mark - Public

- (UIBarButtonItem*)doneButton {
  return nil;
}

- (void)cleanUpSettings {
  
}

- (void)closeSettings {
  
}

- (void)popViewControllerOrCloseSettingsAnimated:(BOOL)animated {
  
}







#pragma mark - ApplicationSettingsCommands

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showAccountsSettingsFromViewController:
    (UIViewController*)baseViewController {

}

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showGoogleServicesSettingsFromViewController:
    (UIViewController*)baseViewController {

}

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showSyncSettingsFromViewController:
    (UIViewController*)baseViewController {

}

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showSyncPassphraseSettingsFromViewController:
    (UIViewController*)baseViewController {

}

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showSavedPasswordsSettingsFromViewController:
            (UIViewController*)baseViewController
                                    showCancelButton:(BOOL)showCancelButton {

}

- (void)showSavedPasswordsSettingsAndStartPasswordCheckFromViewController:
    (UIViewController*)baseViewController {
  
}

// TODO(crbug.com/779791) : Do not pass `baseViewController` through dispatcher.
- (void)showProfileSettingsFromViewController:
    (UIViewController*)baseViewController {

}

- (void)showCreditCardSettings {

}

- (void)showDefaultBrowserSettingsFromViewController:
            (UIViewController*)baseViewController {

}

- (void)showClearBrowsingDataSettings {

}

- (void)showSafetyCheckSettingsAndStartSafetyCheck {

}

- (void)showSafeBrowsingSettings {

}

#pragma mark - UIResponder

- (BOOL)canBecomeFirstResponder {
  return YES;
}
@end
