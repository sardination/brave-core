/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_DATA_SOURCE_H_
#define BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_DATA_SOURCE_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "content/public/browser/url_data_source.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class GURL;

namespace base {
class FilePath;
}  // namespace base

namespace playlist {

class PlaylistService;

// A URL data source for chrome://playlist-data/<playlist-id>/{thumbnail,media}/
// resources, for use in webui pages that want to display downloaded
// playlist thumbnail images
class PlaylistDataSource : public content::URLDataSource {
 public:
  explicit PlaylistDataSource(PlaylistService* service);
  PlaylistDataSource(const PlaylistDataSource&) = delete;
  PlaylistDataSource& operator=(const PlaylistDataSource&) = delete;
  ~PlaylistDataSource() override;

  // content::URLDataSource implementation.
  std::string GetSource() override;
  void StartDataRequest(const GURL& url,
                        const content::WebContents::Getter& wc_getter,
                        GotDataCallback got_data_callback) override;
  std::string GetMimeType(const std::string& path) override;
  bool AllowCaching() override;

 private:
  void GetDataFile(const base::FilePath& data_path,
                   GotDataCallback got_data_callback);
  void OnGotDataFile(GotDataCallback got_data_callback,
                     scoped_refptr<base::RefCountedMemory> input);

  raw_ptr<PlaylistService> service_;

  base::WeakPtrFactory<PlaylistDataSource> weak_factory_{this};
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_PLAYLIST_DATA_SOURCE_H_
