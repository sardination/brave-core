// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/app/first_run_app_state_agent.h"

#import "ios/chrome/app/application_delegate/app_state.h"
#import "ios/chrome/browser/policy/policy_watcher_browser_agent.h"
#import "ios/chrome/browser/policy/policy_watcher_browser_agent_observer_bridge.h"
#import "ios/chrome/browser/ui/main/browser_interface_provider.h"
#import "ios/chrome/browser/ui/main/scene_state.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface FirstRunAppAgent ()

@end

@implementation FirstRunAppAgent {

}

- (void)dealloc {

}

#pragma mark - AppStateAgent

- (void)setAppState:(AppState*)appState {

}

#pragma mark - SceneStateObserver

- (void)sceneStateDidDisableUI:(SceneState*)sceneState {

}

#pragma mark - AppStateObserver

- (void)appState:(AppState*)appState
    willTransitionToInitStage:(InitStage)nextInitStage {
  
}

- (void)appState:(AppState*)appState
    didTransitionFromInitStage:(InitStage)previousInitStage {
  
}

- (void)handleFirstRunStage {
  
}

- (void)appState:(AppState*)appState
    firstSceneHasInitializedUI:(SceneState*)sceneState {
  
}

#pragma mark - Getters and Setters

- (id<BrowserInterface>)presentingInterface {
  return nil;
}

#pragma mark - internal

- (void)setUpPolicyWatcher {

}

- (void)tearDownPolicyWatcher {
  
}

- (void)showFirstRunUI {
  
}

// Presents the first run UI to the user.
- (void)showLegacyFirstRunUI {
  
}

// Shows the new first run UI.
- (void)showNewFirstRunUI {
  
}

- (void)handleFirstRunUIWillFinish {
  
}

// All of this can be triggered from will transition from init stage, or did
// transition to init stage, once the rest is done.
- (void)handleFirstRunUIDidFinish {
  
}

#pragma mark - FirstRunCoordinatorDelegate

- (void)willFinishPresentingScreens {
  
}

- (void)didFinishPresentingScreens {
  
}

#pragma mark - PolicyWatcherBrowserAgentObserving

- (void)policyWatcherBrowserAgentNotifySignInDisabled:
    (PolicyWatcherBrowserAgent*)policyWatcher {
  
}

@end
