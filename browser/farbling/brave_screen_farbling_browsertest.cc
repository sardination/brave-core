/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/path_service.h"
#include "base/test/scoped_feature_list.h"
#include "brave/browser/brave_content_browser_client.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "brave/components/constants/brave_paths.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/common/chrome_content_client.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "net/test/embedded_test_server/http_request.h"
#include "third_party/blink/public/common/features.h"

using brave_shields::ControlType;

class BraveScreenFarblingBrowserTest : public InProcessBrowserTest {
 public:
  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();

    content_client_.reset(new ChromeContentClient);
    content::SetContentClient(content_client_.get());
    browser_content_client_.reset(new BraveContentBrowserClient());
    content::SetBrowserClientForTesting(browser_content_client_.get());

    host_resolver()->AddRule("*", "127.0.0.1");
    content::SetupCrossSiteRedirector(embedded_test_server());

    brave::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_data_dir);
    embedded_test_server()->ServeFilesFromDirectory(test_data_dir);

    ASSERT_TRUE(embedded_test_server()->Start());

    top_level_page_url_ = embedded_test_server()->GetURL("a.com", "/");
    farbling_url_ = embedded_test_server()->GetURL("a.com", "/simple.html");
  }

  void TearDown() override {
    browser_content_client_.reset();
    content_client_.reset();
  }

  HostContentSettingsMap* content_settings() {
    return HostContentSettingsMapFactory::GetForProfile(browser()->profile());
  }

  void AllowFingerprinting() {
    brave_shields::SetFingerprintingControlType(
        content_settings(), ControlType::ALLOW, top_level_page_url_);
  }

  void SetFingerprintingDefault() {
    brave_shields::SetFingerprintingControlType(
        content_settings(), ControlType::DEFAULT, top_level_page_url_);
  }

  content::WebContents* contents() const {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  bool NavigateToURLUntilLoadStop(const GURL& url) {
    EXPECT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
    return WaitForLoadStop(contents());
  }

  Browser* OpenPopup(const std::string& script) const {
    content::ExecuteScriptAsync(contents(), script);
    Browser* popup = ui_test_utils::WaitForBrowserToOpen();
    EXPECT_NE(popup, browser());
    auto* popup_contents = popup->tab_strip_model()->GetActiveWebContents();
    EXPECT_TRUE(WaitForRenderFrameReady(popup_contents->GetMainFrame()));
    return popup;
  }

  const GURL& farbling_url() { return farbling_url_; }

 protected:
  base::test::ScopedFeatureList feature_list_;

 private:
  GURL top_level_page_url_;
  GURL farbling_url_;
  std::unique_ptr<ChromeContentClient> content_client_;
  std::unique_ptr<BraveContentBrowserClient> browser_content_client_;
};

class BraveScreenFarblingBrowserTest_DisableFlag
    : public BraveScreenFarblingBrowserTest {
 public:
  BraveScreenFarblingBrowserTest_DisableFlag() {
    feature_list_.InitAndDisableFeature(
        blink::features::kBraveBlockScreenFingerprinting);
  }
};

const gfx::Rect testWindowBounds[] = {
    gfx::Rect(50, 50, 100, 100), gfx::Rect(50, 50, 100, 0),
    gfx::Rect(200, 100, 0, 100), gfx::Rect(-100, -200, 20000, 10000),
    gfx::Rect(0, 0, 0, 0)};

const char* testScreenSizeScripts[] = {
    "window.outerWidth - window.innerWidth",
    "window.outerHeight - window.innerHeight",
    "window.screen.availWidth - window.innerWidth",
    "window.screen.availHeight - window.innerHeight",
    "window.screen.width - window.innerWidth",
    "window.screen.height - window.innerHeight",
};

#define FARBLE_SCREEN_SIZE(disable_flag)                                    \
  {                                                                         \
    for (bool allow_fingerprinting : {false, true}) {                       \
      allow_fingerprinting ? AllowFingerprinting()                          \
                           : SetFingerprintingDefault();                    \
      for (int j = 0; j < static_cast<int>(std::size(testWindowBounds));    \
           ++j) {                                                           \
        browser()->window()->SetBounds(testWindowBounds[j]);                \
        NavigateToURLUntilLoadStop(farbling_url());                         \
        for (int i = 0;                                                     \
             i < static_cast<int>(std::size(testScreenSizeScripts)); ++i) { \
          std::string testScreenSizeScriptsAbs =                            \
              std::string("Math.abs(") + testScreenSizeScripts[i] + ")";    \
          if (allow_fingerprinting || disable_flag) {                       \
            EXPECT_LT(8, EvalJs(contents(), testScreenSizeScriptsAbs));     \
          } else {                                                          \
            EXPECT_GE(8, EvalJs(contents(), testScreenSizeScriptsAbs));     \
          }                                                                 \
        }                                                                   \
      }                                                                     \
    }                                                                       \
  }

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest, FarbleScreenSize)
FARBLE_SCREEN_SIZE(false)

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest_DisableFlag,
                       FarbleScreenSize_DisableFlag)
FARBLE_SCREEN_SIZE(true)

#define PREPARE_TEST_EVENT                                   \
  "let fakeScreenX = 100, fakeScreenY = 200; "               \
  "let fakeClientX = 300, fakeClientY = 400; "               \
  "let testEvent = document.createEvent('MouseEvent'); "     \
  "testEvent.initMouseEvent('click', true, true, window, 1," \
  "fakeScreenX + devicePixelRatio * fakeClientX,"            \
  "fakeScreenY + devicePixelRatio * fakeClientY,"            \
  "fakeClientX, fakeClientY, false, false, false, false, 0, null); "

const char* testWindowPositionScripts[] = {
    "window.screenX", "window.screenY",
    /* window.screen.availLeft is usually 0, so we don't test that here. */
    "window.screen.availTop",
    PREPARE_TEST_EVENT
    "testEvent.screenX - devicePixelRatio * testEvent.clientX",
    PREPARE_TEST_EVENT
    "testEvent.screenY - devicePixelRatio * testEvent.clientY"};

#define FARBLE_WINDOW_POSITION(disable_flag)                                  \
  {                                                                           \
    for (bool allow_fingerprinting : {false, true}) {                         \
      for (int i = 0;                                                         \
           i < static_cast<int>(std::size(testWindowPositionScripts)); ++i) { \
        bool protectionFoundDisabled = false;                                 \
        for (int j = 0; j < static_cast<int>(std::size(testWindowBounds));    \
             ++j) {                                                           \
          browser()->window()->SetBounds(testWindowBounds[j]);                \
          allow_fingerprinting ? AllowFingerprinting()                        \
                               : SetFingerprintingDefault();                  \
          NavigateToURLUntilLoadStop(farbling_url());                         \
          content::EvalJsResult result =                                      \
              EvalJs(contents(), testWindowPositionScripts[i]);               \
          int resultInt = result.value.GetInt();                              \
          if (abs(resultInt) > 8) {                                           \
            protectionFoundDisabled = true;                                   \
          }                                                                   \
        }                                                                     \
        EXPECT_EQ(protectionFoundDisabled,                                    \
                  allow_fingerprinting || disable_flag);                      \
      }                                                                       \
    }                                                                         \
  }

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest, FarbleWindowPosition)
FARBLE_WINDOW_POSITION(false)

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest_DisableFlag,
                       FarbleWindowPosition_DisableFlag)
FARBLE_WINDOW_POSITION(true)

const char* mediaQueryTestScripts[] = {
    "matchMedia(`(max-device-width: ${innerWidth + 8}px) and "
    "(min-device-width: ${innerWidth}px)`).matches",
    "matchMedia(`(max-device-height: ${innerHeight + 8}px) and "
    "(min-device-height: ${innerHeight}px)`).matches"};

#define FARBLE_SCREEN_MEDIA_QUERY(disable_flag)                             \
  {                                                                         \
    for (bool allow_fingerprinting : {false, true}) {                       \
      for (int j = 0; j < static_cast<int>(std::size(testWindowBounds));    \
           ++j) {                                                           \
        browser()->window()->SetBounds(testWindowBounds[j]);                \
        allow_fingerprinting ? AllowFingerprinting()                        \
                             : SetFingerprintingDefault();                  \
        NavigateToURLUntilLoadStop(farbling_url());                         \
        for (int i = 0;                                                     \
             i < static_cast<int>(std::size(mediaQueryTestScripts)); ++i) { \
          EXPECT_EQ(!disable_flag && !allow_fingerprinting,                 \
                    EvalJs(contents(), mediaQueryTestScripts[i]));          \
        }                                                                   \
      }                                                                     \
    }                                                                       \
  }

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest, FarbleScreenMediaQuery)
FARBLE_SCREEN_MEDIA_QUERY(false)

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest_DisableFlag,
                       FarbleScreenMediaQuery_DisableFlag)
FARBLE_SCREEN_MEDIA_QUERY(true)

const gfx::Rect popupParentWindowBounds[] = {
    gfx::Rect(50, 50, 150, 150),
    gfx::Rect(50, 50, 1000, 200),
    gfx::Rect(100, 500, 90, 10000),
};

#define FARBLE_SCREEN_POPUP_POSITION(disable_flag)                          \
  {                                                                         \
    for (bool allow_fingerprinting : {false, true}) {                       \
      for (int j = 0;                                                       \
           j < static_cast<int>(std::size(popupParentWindowBounds)); ++j) { \
        browser()->window()->SetBounds(testWindowBounds[j]);                \
        allow_fingerprinting ? AllowFingerprinting()                        \
                             : SetFingerprintingDefault();                  \
        NavigateToURLUntilLoadStop(farbling_url());                         \
        gfx::Rect parentBounds = browser()->window()->GetBounds();          \
        const char* script =                                                \
            "open('http://d.test/', '', `left=${screen.availLeft + "        \
            "20},top=${screen.availTop + 20},width=${screen.availWidth - "  \
            "40},height=${screen.availHeight - 40}`);";                     \
        Browser* popup = OpenPopup(script);                                 \
        gfx::Rect childBounds = popup->window()->GetBounds();               \
        bool windowPositionProtected =                                      \
            (childBounds.x() < 28 + parentBounds.x()) &&                    \
            (childBounds.y() < 28 + parentBounds.y()) &&                    \
            (childBounds.width() < parentBounds.width()) &&                 \
            (childBounds.height() < parentBounds.height());                 \
        EXPECT_EQ(!windowPositionProtected,                                 \
                  allow_fingerprinting || disable_flag);                    \
      }                                                                     \
    }                                                                       \
  }

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest,
                       FarbleScreenPopupPosition)
FARBLE_SCREEN_POPUP_POSITION(false)

IN_PROC_BROWSER_TEST_F(BraveScreenFarblingBrowserTest_DisableFlag,
                       FarbleScreenPopupPosition_DisableFlag)
FARBLE_SCREEN_POPUP_POSITION(true)
