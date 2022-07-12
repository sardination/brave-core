/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/common/value_conversion_utils.h"

#include <string>
#include <vector>

#include "base/containers/contains.h"
#include "base/json/json_reader.h"
#include "base/values.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/origin.h"

using testing::ElementsAreArray;

namespace brave_wallet {

namespace {

void TestValueToBlockchainTokenFailCases(const base::Value& value,
                                         const std::vector<std::string>& keys) {
  for (const auto& key : keys) {
    auto invalid_value = value.Clone();
    invalid_value.RemoveKey(key);
    EXPECT_FALSE(
        ValueToBlockchainToken(invalid_value, "0x1", mojom::CoinType::ETH))
        << "ValueToBlockchainToken should fail if " << key << " not exists";
  }
}

constexpr char kNetworkDataValue[] = R"(
{
  "chainId": "0x5",
  "chainName": "Goerli",
  "activeRpcEndpointIndex": 1,
  "rpcUrls": [
    "https://goerli.infura.io/v3/INSERT_API_KEY_HERE",
    "https://second.infura.io/"
  ],
  "iconUrls": [
    "https://xdaichain.com/fake/example/url/xdai.svg",
    "https://xdaichain.com/fake/example/url/xdai.png"
  ],
  "nativeCurrency": {
    "name": "Goerli ETH",
    "symbol": "gorETH",
    "decimals": 18
  },
  "blockExplorerUrls": ["https://goerli.etherscan.io"],
  "is_eip1559": true
})";

}  // namespace

TEST(ValueConversionUtilsUnitTest, ParseEip3085Payload) {
  mojom::NetworkInfoPtr chain =
      ParseEip3085Payload(base::JSONReader::Read(kNetworkDataValue).value());
  ASSERT_TRUE(chain);
  EXPECT_EQ("0x5", chain->chain_id);
  EXPECT_EQ("Goerli", chain->chain_name);
  EXPECT_EQ(size_t(2), chain->rpc_endpoints.size());
  EXPECT_THAT(
      chain->rpc_endpoints,
      ElementsAreArray({GURL("https://goerli.infura.io/v3/INSERT_API_KEY_HERE"),
                        GURL("https://second.infura.io/")}));
  EXPECT_EQ("https://goerli.etherscan.io", chain->block_explorer_urls.front());
  EXPECT_EQ("Goerli ETH", chain->symbol_name);
  EXPECT_EQ("gorETH", chain->symbol);
  EXPECT_EQ(18, chain->decimals);
  EXPECT_THAT(
      chain->icon_urls,
      ElementsAreArray({"https://xdaichain.com/fake/example/url/xdai.svg",
                        "https://xdaichain.com/fake/example/url/xdai.png"}));
  ASSERT_EQ(chain->coin, mojom::CoinType::ETH);

  // is_eip1559 is not parsed for Eip3085.
  EXPECT_FALSE(chain->is_eip1559);

  // active_rpc_endpoint_index is not parsed for Eip3085.
  EXPECT_EQ(0, chain->active_rpc_endpoint_index);
}

TEST(ValueConversionUtilsUnitTest, ValueToEthNetworkInfoTest) {
  {
    mojom::NetworkInfoPtr chain = ValueToEthNetworkInfo(
        base::JSONReader::Read(kNetworkDataValue).value());
    ASSERT_TRUE(chain);
    EXPECT_EQ("0x5", chain->chain_id);
    EXPECT_EQ("Goerli", chain->chain_name);
    EXPECT_EQ(size_t(2), chain->rpc_endpoints.size());
    EXPECT_THAT(chain->rpc_endpoints,
                ElementsAreArray(
                    {GURL("https://goerli.infura.io/v3/INSERT_API_KEY_HERE"),
                     GURL("https://second.infura.io/")}));
    EXPECT_EQ(1, chain->active_rpc_endpoint_index);
    EXPECT_EQ("https://goerli.etherscan.io",
              chain->block_explorer_urls.front());
    EXPECT_EQ("Goerli ETH", chain->symbol_name);
    EXPECT_EQ("gorETH", chain->symbol);
    EXPECT_EQ(18, chain->decimals);
    EXPECT_THAT(
        chain->icon_urls,
        ElementsAreArray({"https://xdaichain.com/fake/example/url/xdai.svg",
                          "https://xdaichain.com/fake/example/url/xdai.png"}));
    ASSERT_EQ(chain->coin, mojom::CoinType::ETH);
    EXPECT_TRUE(chain->is_eip1559);
  }
  {
    mojom::NetworkInfoPtr chain =
        ValueToEthNetworkInfo(base::JSONReader::Read(R"({
      "chainId": "0x5"
    })")
                                  .value());
    ASSERT_TRUE(chain);
    EXPECT_EQ("0x5", chain->chain_id);
    ASSERT_TRUE(chain->chain_name.empty());
    ASSERT_TRUE(chain->rpc_endpoints.empty());
    ASSERT_TRUE(chain->icon_urls.empty());
    ASSERT_TRUE(chain->block_explorer_urls.empty());
    ASSERT_TRUE(chain->symbol_name.empty());
    ASSERT_TRUE(chain->symbol.empty());
    ASSERT_EQ(chain->coin, mojom::CoinType::ETH);
    ASSERT_FALSE(chain->is_eip1559);
    EXPECT_EQ(chain->decimals, 0);
  }

  {
    mojom::NetworkInfoPtr chain =
        ValueToEthNetworkInfo(base::JSONReader::Read(R"({})").value());
    ASSERT_FALSE(chain);
  }
  {
    mojom::NetworkInfoPtr chain =
        ValueToEthNetworkInfo(base::JSONReader::Read(R"([])").value());
    ASSERT_FALSE(chain);
  }
}

TEST(ValueConversionUtilsUnitTest, EthNetworkInfoToValueTest) {
  mojom::NetworkInfo chain(
      "chain_id", "chain_name", {"https://url1.com"}, {"https://url1.com"}, 1,
      {GURL("https://url1.com"), GURL("https://url123.com")}, "symbol_name",
      "symbol", 11, mojom::CoinType::ETH, true);
  base::Value value = brave_wallet::EthNetworkInfoToValue(chain);
  EXPECT_EQ(*value.FindStringKey("chainId"), chain.chain_id);
  EXPECT_EQ(*value.FindStringKey("chainName"), chain.chain_name);
  EXPECT_EQ(*value.FindStringPath("nativeCurrency.name"), chain.symbol_name);
  EXPECT_EQ(*value.FindStringPath("nativeCurrency.symbol"), chain.symbol);
  EXPECT_EQ(*value.FindIntPath("nativeCurrency.decimals"), chain.decimals);
  EXPECT_EQ(value.FindBoolKey("is_eip1559").value(), true);
  for (const auto& entry : value.FindListKey("rpcUrls")->GetList()) {
    ASSERT_TRUE(base::Contains(chain.rpc_endpoints, entry.GetString()));
  }
  EXPECT_EQ(value.FindIntKey("activeRpcEndpointIndex").value(), 1);

  for (const auto& entry : value.FindListKey("iconUrls")->GetList()) {
    ASSERT_NE(std::find(chain.icon_urls.begin(), chain.icon_urls.end(),
                        entry.GetString()),
              chain.icon_urls.end());
  }
  auto* blocked_urls = value.FindListKey("blockExplorerUrls");
  for (const auto& entry : blocked_urls->GetList()) {
    ASSERT_NE(std::find(chain.block_explorer_urls.begin(),
                        chain.block_explorer_urls.end(), entry.GetString()),
              chain.block_explorer_urls.end());
  }

  auto result = ValueToEthNetworkInfo(value);
  ASSERT_TRUE(result->Equals(chain));
}

TEST(ValueConversionUtilsUnitTest, ValueToBlockchainToken) {
  absl::optional<base::Value> json_value = base::JSONReader::Read(R"({
      "address": "0x0D8775F648430679A709E98d2b0Cb6250d2887EF",
      "name": "Basic Attention Token",
      "symbol": "BAT",
      "logo": "bat.png",
      "is_erc20": true,
      "is_erc721": false,
      "decimals": 18,
      "visible": true,
      "token_id": "",
      "coingecko_id": ""
  })");
  ASSERT_TRUE(json_value);

  mojom::BlockchainTokenPtr expected_token = mojom::BlockchainToken::New(
      "0x0D8775F648430679A709E98d2b0Cb6250d2887EF", "Basic Attention Token",
      "bat.png", true, false, "BAT", 18, true, "", "", "0x1",
      mojom::CoinType::ETH);

  mojom::BlockchainTokenPtr token =
      ValueToBlockchainToken(json_value.value(), "0x1", mojom::CoinType::ETH);
  EXPECT_EQ(token, expected_token);

  // Test input value with required keys.
  TestValueToBlockchainTokenFailCases(
      json_value.value(), {"address", "name", "symbol", "is_erc20", "is_erc721",
                           "decimals", "visible"});

  // Test input value with optional keys.
  base::Value optional_value = json_value.value().Clone();
  optional_value.RemoveKey("logo");
  optional_value.RemoveKey("token_id");
  optional_value.RemoveKey("coingecko_id");
  expected_token->logo = "";
  token = ValueToBlockchainToken(optional_value, "0x1", mojom::CoinType::ETH);
  EXPECT_EQ(token, expected_token);
}

TEST(ValueConversionUtilsUnitTest, PermissionRequestResponseToValue) {
  url::Origin origin = url::Origin::Create(GURL("https://brave.com"));
  std::vector<std::string> accounts{
      "0xA99D71De40D67394eBe68e4D0265cA6C9D421029"};
  base::Value value = PermissionRequestResponseToValue(origin, accounts);

  // [{
  //   "caveats":[
  //     {
  //       "name":"primaryAccountOnly",
  //        "type":"limitResponseLength",
  //        "value":1
  //     }, {
  //       "name":"exposedAccounts",
  //       "type":"filterResponse",
  //       "value": ["0xA99D71De40D67394eBe68e4D0265cA6C9D421029"]
  //     }
  //   ],
  //   "context":[
  //     "https://github.com/MetaMask/rpc-cap"
  //   ],
  //   "date":1.637594791027276e+12,
  //   "id":"2485c0da-2131-4801-9918-26e8de929a29",
  //   "invoker":"https://brave.com",
  //   "parentCapability":"eth_accounts"
  // }]"

  base::ListValue* list_value;
  ASSERT_TRUE(value.GetAsList(&list_value));
  ASSERT_EQ(list_value->GetList().size(), 1UL);

  base::Value& param0 = list_value->GetList()[0];
  base::Value* caveats = param0.FindListPath("caveats");
  ASSERT_NE(caveats, nullptr);
  ASSERT_EQ(caveats->GetList().size(), 2UL);

  base::Value& caveats0 = caveats->GetList()[0];
  std::string* name = caveats0.FindStringKey("name");
  ASSERT_NE(name, nullptr);
  EXPECT_EQ(*name, "primaryAccountOnly");
  std::string* type = caveats0.FindStringKey("type");
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "limitResponseLength");
  absl::optional<int> primary_accounts_only_value =
      caveats0.FindIntKey("value");
  ASSERT_NE(primary_accounts_only_value, absl::nullopt);
  EXPECT_EQ(*primary_accounts_only_value, 1);

  base::Value& caveats1 = caveats->GetList()[1];
  name = caveats1.FindStringKey("name");
  ASSERT_NE(name, nullptr);
  EXPECT_EQ(*name, "exposedAccounts");
  type = caveats1.FindStringKey("type");
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "filterResponse");
  base::Value* exposed_accounts = caveats1.FindListKey("value");
  ASSERT_NE(exposed_accounts, nullptr);
  ASSERT_EQ(exposed_accounts->GetList().size(), 1UL);
  EXPECT_EQ(exposed_accounts->GetList()[0],
            base::Value("0xA99D71De40D67394eBe68e4D0265cA6C9D421029"));

  base::Value* context = param0.FindListPath("context");
  ASSERT_NE(context, nullptr);
  ASSERT_EQ(context->GetList().size(), 1UL);
  EXPECT_EQ(context->GetList()[0],
            base::Value("https://github.com/MetaMask/rpc-cap"));

  absl::optional<double> date = param0.FindDoubleKey("date");
  ASSERT_NE(date, absl::nullopt);

  std::string* id = param0.FindStringKey("id");
  ASSERT_NE(id, nullptr);

  std::string* invoker = param0.FindStringKey("invoker");
  ASSERT_NE(invoker, nullptr);
  EXPECT_EQ(*invoker, "https://brave.com");

  std::string* parent_capability = param0.FindStringKey("parentCapability");
  ASSERT_NE(parent_capability, nullptr);
  EXPECT_EQ(*parent_capability, "eth_accounts");
}

TEST(ValueConversionUtilsUnitTest, GetFirstValidChainURL) {
  std::vector<GURL> urls = {
      GURL("https://goerli.infura.io/v3/${INFURA_API_KEY}"),
      GURL("https://goerli.alchemy.io/v3/${ALCHEMY_API_KEY}"),
      GURL("https://goerli.apikey.io/v3/${API_KEY}"),
      GURL("https://goerli.apikey.io/v3/${PULSECHAIN_API_KEY}"),
      GURL("wss://goerli.infura.io/v3/")};

  // Uses the first URL when a good URL is not available
  auto index = GetFirstValidChainURLIndex(urls);
  EXPECT_EQ(index, 0);

  urls.emplace_back("https://goerli.infura.io/v3/rpc");
  urls.emplace_back("https://goerli.infura.io/v3/rpc2");
  // Uses the first HTTP(S) URL without a variable when possible
  index = GetFirstValidChainURLIndex(urls);
  EXPECT_EQ(index, 5);

  // Empty URL spec list returns 0
  EXPECT_EQ(GetFirstValidChainURLIndex(std::vector<GURL>()), 0);
}

}  // namespace brave_wallet
