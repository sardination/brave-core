/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_adaptive_captcha/post_adaptive_captcha_solution.h"

#include <memory>
#include <string>

#include "base/run_loop.h"
#include "base/test/task_environment.h"
#include "brave/components/api_request_helper/api_request_helper.h"
#include "brave/components/brave_adaptive_captcha/server_util.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "services/data_decoder/public/cpp/test_support/in_process_data_decoder.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=PostAdaptiveCaptchaSolutionTest.*

namespace brave_adaptive_captcha {

class PostAdaptiveCaptchaSolutionTest : public testing::Test {
 public:
  PostAdaptiveCaptchaSolutionTest()
      : api_request_helper_(TRAFFIC_ANNOTATION_FOR_TESTS,
                            test_url_loader_factory_.GetSafeWeakWrapper()),
        post_solution_(std::make_unique<PostAdaptiveCaptchaSolution>(
            &api_request_helper_)) {
    brave_adaptive_captcha::SetServerHostForTesting(
        "https://grants.rewards.brave.com");
  }

  void OnPostSolutionServerOK(const std::string& nonce) {
    EXPECT_EQ(nonce, "0123456789");
    SignalUrlLoadCompleted();
  }

  void OnPostSolutionServerError404(const std::string& nonce) {
    EXPECT_EQ(nonce, "");
    SignalUrlLoadCompleted();
  }

  void OnPostSolutionServerError500(const std::string& nonce) {
    EXPECT_EQ(nonce, "");
    SignalUrlLoadCompleted();
  }

  void OnPostSolutionServerErrorRandom(const std::string& nonce) {
    EXPECT_EQ(nonce, "");
    SignalUrlLoadCompleted();
  }

 protected:
  network::TestURLLoaderFactory test_url_loader_factory_;
  api_request_helper::APIRequestHelper api_request_helper_;
  std::unique_ptr<PostAdaptiveCaptchaSolution> post_solution_;

  void WaitForUrlLoadToComplete() {
    if (url_loaded_) {
      return;
    }

    run_loop_ = std::make_unique<base::RunLoop>();
    run_loop_->Run();
  }

 private:
  base::test::TaskEnvironment scoped_task_environment_;
  data_decoder::test::InProcessDataDecoder in_process_data_decoder_;
  std::unique_ptr<base::RunLoop> run_loop_;
  bool url_loaded_ = false;

  void SignalUrlLoadCompleted() {
    url_loaded_ = true;
    if (run_loop_) {
      run_loop_->Quit();
    }
  }
};

TEST_F(PostAdaptiveCaptchaSolutionTest, ServerOK) {
  test_url_loader_factory_.AddResponse(
      "https://grants.rewards.brave.com"
      "/v3/captcha/solution/payment_id/captcha_id",
      "{ \"solution\": \"0123456789\" }", net::HTTP_OK);
  post_solution_->Request(
      "payment_id", "captcha_id",
      base::BindOnce(&PostAdaptiveCaptchaSolutionTest::OnPostSolutionServerOK,
                     base::Unretained(this)));
  WaitForUrlLoadToComplete();
}

TEST_F(PostAdaptiveCaptchaSolutionTest, ServerError404) {
  test_url_loader_factory_.AddResponse(
      "https://grants.rewards.brave.com"
      "/v3/captcha/solution/payment_id/captcha_id",
      "", net::HTTP_NOT_FOUND);
  post_solution_->Request(
      "payment_id", "captcha_id",
      base::BindOnce(
          &PostAdaptiveCaptchaSolutionTest::OnPostSolutionServerError404,
          base::Unretained(this)));
  WaitForUrlLoadToComplete();
}

TEST_F(PostAdaptiveCaptchaSolutionTest, ServerError500) {
  test_url_loader_factory_.AddResponse(
      "https://grants.rewards.brave.com"
      "/v3/captcha/solution/payment_id/captcha_id",
      "", net::HTTP_INTERNAL_SERVER_ERROR);
  post_solution_->Request(
      "payment_id", "captcha_id",
      base::BindOnce(
          &PostAdaptiveCaptchaSolutionTest::OnPostSolutionServerError500,
          base::Unretained(this)));
  WaitForUrlLoadToComplete();
}

TEST_F(PostAdaptiveCaptchaSolutionTest, ServerErrorRandom) {
  test_url_loader_factory_.AddResponse(
      "https://grants.rewards.brave.com"
      "/v3/captcha/solution/payment_id/captcha_id",
      "", net::HTTP_TOO_MANY_REQUESTS);
  post_solution_->Request(
      "payment_id", "captcha_id",
      base::BindOnce(
          &PostAdaptiveCaptchaSolutionTest::OnPostSolutionServerErrorRandom,
          base::Unretained(this)));
  WaitForUrlLoadToComplete();
}

}  // namespace brave_adaptive_captcha
