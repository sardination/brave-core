/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADAPTIVE_CAPTCHA_POST_ADAPTIVE_CAPTCHA_SOLUTION_H_
#define BRAVE_COMPONENTS_BRAVE_ADAPTIVE_CAPTCHA_POST_ADAPTIVE_CAPTCHA_SOLUTION_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/containers/flat_map.h"
#include "base/memory/raw_ptr.h"

// GET /v3/captcha/solution/{payment_id}/{captcha_id}
//
// Success code:
// HTTP_OK (200)
//
// Error codes:
// HTTP_NOT_FOUND (404)
// HTTP_INTERNAL_SERVER_ERROR (500)
//
// Response body:
// {
//   "solution": "<attestation_nonce>"
// }

namespace api_request_helper {
class APIRequestHelper;
}

namespace brave_adaptive_captcha {

using OnPostAdaptiveCaptchaSolution =
    base::OnceCallback<void(const std::string&)>;

class PostAdaptiveCaptchaSolution {
 public:
  explicit PostAdaptiveCaptchaSolution(
      api_request_helper::APIRequestHelper* api_request_helper);
  ~PostAdaptiveCaptchaSolution();

  void Request(const std::string& payment_id,
               const std::string& captcha_id,
               OnPostAdaptiveCaptchaSolution callback);

 private:
  std::string GetUrl(const std::string& payment_id,
                     const std::string& captcha_id);

  bool CheckStatusCode(int status_code);

  bool ParseBody(const std::string& body, std::string* nonce);

  void OnResponse(
      OnPostAdaptiveCaptchaSolution callback,
      int response_code,
      const std::string& response_body,
      const base::flat_map<std::string, std::string>& response_headers);

  raw_ptr<api_request_helper::APIRequestHelper> api_request_helper_ = nullptr;
};

}  // namespace brave_adaptive_captcha

#endif  // BRAVE_COMPONENTS_BRAVE_ADAPTIVE_CAPTCHA_POST_ADAPTIVE_CAPTCHA_SOLUTION_H_
