/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_INSTALL_STATIC_INSTALL_DETAILS_H_
#define BRAVE_CHROMIUM_SRC_CHROME_INSTALL_STATIC_INSTALL_DETAILS_H_

#define is_primary_mode                      \
  is_primary_mode_unused() { return false; } \
  static bool IsSet();                       \
  bool is_primary_mode

#include "src/chrome/install_static/install_details.h"
#undef is_primary_mode

#endif  // BRAVE_CHROMIUM_SRC_CHROME_INSTALL_STATIC_INSTALL_DETAILS_H_
