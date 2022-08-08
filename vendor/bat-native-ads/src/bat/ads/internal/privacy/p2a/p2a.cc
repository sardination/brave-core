/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/privacy/p2a/p2a.h"

#include <utility>

#include "base/values.h"
#include "bat/ads/internal/ads_client_helper.h"

namespace ads {
namespace privacy {
namespace p2a {

void RecordEvent(const std::string& name,
                 const std::vector<std::string>& questions) {
  DCHECK(!name.empty());
  DCHECK(!questions.empty());

  base::Value::List list;
  for (const auto& question : questions) {
    list.Append(question);
  }

  AdsClientHelper::GetInstance()->RecordP2AEvent(name, std::move(list));
}

}  // namespace p2a
}  // namespace privacy
}  // namespace ads
