/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlist/playlist_tab_helper.h"

#include "base/feature_list.h"
#include "brave/components/playlist/features.h"

namespace playlist {

PlaylistTabHelper::PlaylistTabHelper(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      content::WebContentsUserData<PlaylistTabHelper>(*contents) {}

PlaylistTabHelper::~PlaylistTabHelper() = default;

// static
void PlaylistTabHelper::HideMediaSrcAPIFromContents(
    content::WebContents* contents) {
  if (!base::FeatureList::IsEnabled(features::kPlaylist))
    return;

  DCHECK(contents && contents->GetMainFrame());

  // This script is from
  // https://github.com/brave/brave-ios/blob/development/Client/Frontend/UserContent/UserScripts/PlaylistSwizzler.js
  static const std::u16string kScriptToHideMediaSourceAPI =
      uR"-(
    (function() {
      // Stub out the MediaSource API so video players do not attempt to use `blob` for streaming
      if (window.MediaSource || window.WebKitMediaSource || window.HTMLMediaElement && HTMLMediaElement.prototype.webkitSourceAddId) {
        window.MediaSource = null;
        window.WebKitMediaSource = null;
        delete window.MediaSource;
        delete window.WebKitMediaSource;
      }
    })();
    )-";

  const auto execute_script_callback = [](std::u16string script,
                                          content::RenderFrameHost* frame) {
#if BUILDFLAG(IS_ANDROID)
    frame->ExecuteJavaScript(kScriptToHideMediaSourceAPI, base::NullCallback());
#else
    // In order to hide js API from main world, use testing api temporarily.
    // TODO(sko) On Dekstop, it's not allowed to inject script like this.
    // should we find another way?
    frame->ExecuteJavaScriptForTests(kScriptToHideMediaSourceAPI,
                                     base::NullCallback());
#endif
  };

  contents->GetMainFrame()->ForEachRenderFrameHost(base::BindRepeating(
      execute_script_callback, kScriptToHideMediaSourceAPI));
}

void PlaylistTabHelper::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  HideMediaSrcAPIFromContents(web_contents());
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(PlaylistTabHelper);

}  // namespace playlist
