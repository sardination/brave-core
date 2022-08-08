/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_

#include "bat/ledger/internal/endpoint/endpoint_base.h"
#include "bat/ledger/ledger_client.h"

template <typename>
struct CallbackTypeImpl;

template <typename R, typename P1, typename... Ps>
struct CallbackTypeImpl<R (*)(P1, Ps...)> {
  using type = P1;
};

template <typename T>
using CallbackType = typename CallbackTypeImpl<T>::type;

namespace ledger {
class LedgerImpl;

namespace endpoint {

template <type::UrlMethod method, typename Impl>
class Endpoint : public EndpointBase {
 public:
  Endpoint(LedgerImpl* ledger) : EndpointBase(ledger) {}

  virtual ~Endpoint() = default;

  virtual std::string Url() = 0;
  virtual std::vector<std::string> Headers() { return {}; }
  virtual std::string Content() { return {}; }
  virtual std::string ContentType() {
    return "application/json; charset=utf-8";
  }
  virtual bool SkipLog() { return {}; }
  virtual uint32_t LoadFlags() { return {}; }

  template <typename Callback>
  void Request(Callback callback) {
    auto request = type::UrlRequest::New();
    request->url = Url();
    request->method = method;
    request->headers = Headers();
    request->content = Content();
    request->content_type = ContentType();
    request->skip_log = SkipLog();
    request->load_flags = LoadFlags();

    static_assert(
        std::is_same_v<Callback, CallbackType<decltype(&Impl::OnLoadURL)>>);
    LoadURL(std::move(request),
            base::BindOnce(&Impl::OnLoadURL, std::move(callback)));
  }
};

}  // namespace endpoint
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_
