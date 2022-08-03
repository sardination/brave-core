/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_TAB_HELPER_H_
#define BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_TAB_HELPER_H_

#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace playlist {

class PlaylistTabHelper
    : public content::WebContentsObserver,
      public content::WebContentsUserData<PlaylistTabHelper> {
 public:
  explicit PlaylistTabHelper(content::WebContents* contents);
  ~PlaylistTabHelper() override;

  static void HideMediaSrcAPIFromContents(content::WebContents* contents);

  // content::WebContentsObserver overrides:
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;

 private:
  friend class content::WebContentsUserData<PlaylistTabHelper>;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_TAB_HELPER_H_
