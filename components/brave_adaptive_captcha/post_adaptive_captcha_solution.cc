/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_adaptive_captcha/post_adaptive_captcha_solution.h"

#include <string>
#include <utility>

#include "base/check.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "brave/components/api_request_helper/api_request_helper.h"
#include "brave/components/brave_adaptive_captcha/server_util.h"
#include "net/http/http_status_code.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "url/gurl.h"

namespace brave_adaptive_captcha {

PostAdaptiveCaptchaSolution::PostAdaptiveCaptchaSolution(
    api_request_helper::APIRequestHelper* api_request_helper)
    : api_request_helper_(api_request_helper) {
  DCHECK(api_request_helper_);
}

PostAdaptiveCaptchaSolution::~PostAdaptiveCaptchaSolution() = default;

std::string PostAdaptiveCaptchaSolution::GetUrl(const std::string& payment_id,
                                                const std::string& captcha_id) {
  const std::string path = base::StringPrintf(
      "/v3/captcha/solution/%s/%s", payment_id.c_str(), captcha_id.c_str());

  return GetServerUrl(path);
}

bool PostAdaptiveCaptchaSolution::CheckStatusCode(int status_code) {
  if (status_code == net::HTTP_NOT_FOUND) {
    VLOG(1) << "No captcha scheduled for given payment id";
    return false;
  }

  if (status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    LOG(ERROR) << "Failed to post the captcha solution";
    return false;
  }

  if (status_code != net::HTTP_OK) {
    LOG(ERROR) << "Unexpected HTTP status: " << status_code;
    return false;
  }

  return true;
}

bool PostAdaptiveCaptchaSolution::ParseBody(const std::string& body,
                                            std::string* nonce) {
  DCHECK(nonce);

  absl::optional<base::Value> value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    LOG(ERROR) << "Invalid JSON";
    return false;
  }

  const auto& dict = value->GetDict();
  const std::string* nonce_value = dict.FindString("solution");
  if (!nonce_value) {
    LOG(ERROR) << "Missing solution";
    return false;
  }

  *nonce = *nonce_value;

  return true;
}

void PostAdaptiveCaptchaSolution::Request(
    const std::string& payment_id,
    const std::string& captcha_id,
    OnPostAdaptiveCaptchaSolution callback) {
  auto api_request_helper_callback =
      base::BindOnce(&PostAdaptiveCaptchaSolution::OnResponse,
                     base::Unretained(this), std::move(callback));
  api_request_helper_->Request("POST", GURL(GetUrl(payment_id, captcha_id)), "",
                               "", false,
                               std::move(api_request_helper_callback));
}

void PostAdaptiveCaptchaSolution::OnResponse(
    OnPostAdaptiveCaptchaSolution callback,
    int response_code,
    const std::string& response_body,
    const base::flat_map<std::string, std::string>& response_headers) {
  bool result = CheckStatusCode(response_code);
  if (!result) {
    std::move(callback).Run("");
    return;
  }

  std::string nonce;
  result = ParseBody(response_body, &nonce);
  if (!result) {
    std::move(callback).Run("");
    return;
  }

  std::move(callback).Run(nonce);
}

}  // namespace brave_adaptive_captcha
