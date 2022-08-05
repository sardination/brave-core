// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

const urlParam = new URLSearchParams(window.location.search).get('targetUrl')
const imgUrl = urlParam?.replace('chrome://image?', '') || ''

const imageElement = document.getElementById('untrusted-image') as HTMLImageElement | null

if (imageElement && imgUrl) {
  imageElement.onload = function () {
    const loaderElement = document.getElementById('loader') as HTMLDivElement | null
    if (loaderElement) {
      loaderElement.className = loaderElement.className + ' hidden'
    }
  }
  // Defer loading the image until it reaches a calculated distance from the
  // viewport, as defined by the browser.
  // Ref: https://web.dev/browser-level-image-lazy-loading
  imageElement.loading = 'lazy'
  imageElement.src = imgUrl
}
