/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/common/value_conversion_utils.h"

#include <string>
#include <vector>

#include "base/json/json_reader.h"
#include "base/values.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/origin.h"

namespace brave_wallet {

namespace {

void TestValueToBlockchainTokenFailCases(const base::Value::Dict& value,
                                         const std::vector<std::string>& keys) {
  for (const auto& key : keys) {
    auto invalid_value = value.Clone();
    invalid_value.Remove(key);
    EXPECT_FALSE(
        ValueToBlockchainToken(invalid_value, "0x1", mojom::CoinType::ETH))
        << "ValueToBlockchainToken should fail if " << key << " not exists";
  }
}

}  // namespace

TEST(ValueConversionUtilsUnitTest, ValueToEthNetworkInfoTest) {
  {
    auto value = base::JSONReader::Read(R"({
      "chainId": "0x5",
      "chainName": "Goerli",
      "rpcUrls": [
        "ftp://bar/",
        "ftp://localhost/",
        "http://bar/",
        "http://localhost/",
        "http://127.0.0.1/",
        "https://goerli.infura.io/v3/INSERT_API_KEY_HERE",
        "https://second.infura.io/",
        []
      ],
      "iconUrls": [
        "ftp://bar/",
        "ftp://localhost/",
        "http://bar/",
        "http://localhost/",
        "http://127.0.0.1/",
        "https://xdaichain.com/fake/example/url/xdai.svg",
        "https://xdaichain.com/fake/example/url/xdai.png",
        {}
      ],
      "nativeCurrency": {
        "name": "Goerli ETH",
        "symbol": "gorETH",
        "decimals": 18
      },
      "blockExplorerUrls": [
        "ftp://bar/",
        "ftp://localhost/",
        "http://bar/",
        "http://localhost/",
        "http://127.0.0.1/",
        "https://goerli.etherscan.io",
        2
      ],
      "is_eip1559": true
    })")
                     .value();
    mojom::NetworkInfoPtr chain =
        brave_wallet::ValueToEthNetworkInfo(value, true);
    mojom::NetworkInfoPtr chain2 =
        brave_wallet::ValueToEthNetworkInfo(value, false);
    ASSERT_TRUE(chain);
    ASSERT_TRUE(chain2);
    EXPECT_EQ("0x5", chain->chain_id);
    EXPECT_EQ("Goerli", chain->chain_name);
    EXPECT_EQ(chain->rpc_urls,
              std::vector<std::string>(
                  {"http://localhost/", "http://127.0.0.1/",
                   "https://goerli.infura.io/v3/INSERT_API_KEY_HERE",
                   "https://second.infura.io/"}));
    EXPECT_EQ(chain2->rpc_urls,
              std::vector<std::string>(
                  {"ftp://bar/", "ftp://localhost/", "http://bar/",
                   "http://localhost/", "http://127.0.0.1/",
                   "https://goerli.infura.io/v3/INSERT_API_KEY_HERE",
                   "https://second.infura.io/"}));
    EXPECT_EQ(
        chain->block_explorer_urls,
        std::vector<std::string>({"http://localhost/", "http://127.0.0.1/",
                                  "https://goerli.etherscan.io"}));
    EXPECT_EQ(chain2->block_explorer_urls,
              std::vector<std::string>({"ftp://bar/", "ftp://localhost/",
                                        "http://bar/", "http://localhost/",
                                        "http://127.0.0.1/",
                                        "https://goerli.etherscan.io"}));
    EXPECT_EQ("Goerli ETH", chain->symbol_name);
    EXPECT_EQ("gorETH", chain->symbol);
    EXPECT_EQ(18, chain->decimals);
    EXPECT_EQ(chain->icon_urls,
              std::vector<std::string>(
                  {"http://localhost/", "http://127.0.0.1/",
                   "https://xdaichain.com/fake/example/url/xdai.svg",
                   "https://xdaichain.com/fake/example/url/xdai.png"}));
    EXPECT_EQ(chain2->icon_urls,
              std::vector<std::string>(
                  {"ftp://bar/", "ftp://localhost/", "http://bar/",
                   "http://localhost/", "http://127.0.0.1/",
                   "https://xdaichain.com/fake/example/url/xdai.svg",
                   "https://xdaichain.com/fake/example/url/xdai.png"}));
    ASSERT_EQ(chain->coin, mojom::CoinType::ETH);
    ASSERT_TRUE(chain->data);
    ASSERT_TRUE(chain->data->is_eth_data());
    EXPECT_TRUE(chain->data->get_eth_data()->is_eip1559);
  }
  {
    mojom::NetworkInfoPtr chain =
        brave_wallet::ValueToEthNetworkInfo(base::JSONReader::Read(R"({
      "chainId": "0x5"
    })")
                                                .value());
    ASSERT_TRUE(chain);
    EXPECT_EQ("0x5", chain->chain_id);
    ASSERT_TRUE(chain->chain_name.empty());
    ASSERT_TRUE(chain->rpc_urls.empty());
    ASSERT_TRUE(chain->icon_urls.empty());
    ASSERT_TRUE(chain->block_explorer_urls.empty());
    ASSERT_TRUE(chain->symbol_name.empty());
    ASSERT_TRUE(chain->symbol.empty());
    ASSERT_EQ(chain->coin, mojom::CoinType::ETH);
    ASSERT_FALSE(chain->data);
    EXPECT_EQ(chain->decimals, 0);
  }

  {
    mojom::NetworkInfoPtr chain =
        brave_wallet::ValueToEthNetworkInfo(base::JSONReader::Read(R"({
    })")
                                                .value());
    ASSERT_FALSE(chain);
  }
  {
    mojom::NetworkInfoPtr chain =
        brave_wallet::ValueToEthNetworkInfo(base::JSONReader::Read(R"([
          ])")
                                                .value());
    ASSERT_FALSE(chain);
  }
}

TEST(ValueConversionUtilsUnitTest, EthNetworkInfoToValueTest) {
  mojom::NetworkInfo chain(
      "chain_id", "chain_name", {"https://url1.com"}, {"https://url1.com"},
      {"https://url1.com"}, "symbol_name", "symbol", 11, mojom::CoinType::ETH,
      mojom::NetworkInfoData::NewEthData(mojom::NetworkInfoDataETH::New(true)));
  base::Value::Dict value = brave_wallet::EthNetworkInfoToValue(chain);
  EXPECT_EQ(*value.FindString("chainId"), chain.chain_id);
  EXPECT_EQ(*value.FindString("chainName"), chain.chain_name);
  EXPECT_EQ(*value.FindStringByDottedPath("nativeCurrency.name"),
            chain.symbol_name);
  EXPECT_EQ(*value.FindStringByDottedPath("nativeCurrency.symbol"),
            chain.symbol);
  EXPECT_EQ(*value.FindIntByDottedPath("nativeCurrency.decimals"),
            chain.decimals);
  EXPECT_EQ(value.FindBool("is_eip1559").value(), true);
  auto* rpc_urls = value.FindList("rpcUrls");
  for (const auto& entry : *rpc_urls) {
    ASSERT_NE(std::find(chain.rpc_urls.begin(), chain.rpc_urls.end(),
                        entry.GetString()),
              chain.rpc_urls.end());
  }

  for (const auto& entry : *value.FindList("iconUrls")) {
    ASSERT_NE(std::find(chain.icon_urls.begin(), chain.icon_urls.end(),
                        entry.GetString()),
              chain.icon_urls.end());
  }
  auto* blocked_urls = value.FindList("blockExplorerUrls");
  for (const auto& entry : *blocked_urls) {
    ASSERT_NE(std::find(chain.block_explorer_urls.begin(),
                        chain.block_explorer_urls.end(), entry.GetString()),
              chain.block_explorer_urls.end());
  }

  auto result = ValueToEthNetworkInfo(base::Value(value.Clone()));
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

  mojom::BlockchainTokenPtr token = ValueToBlockchainToken(
      json_value->GetDict(), "0x1", mojom::CoinType::ETH);
  EXPECT_EQ(token, expected_token);

  // Test input value with required keys.
  TestValueToBlockchainTokenFailCases(json_value->GetDict(),
                                      {"address", "name", "symbol", "is_erc20",
                                       "is_erc721", "decimals", "visible"});

  // Test input value with optional keys.
  base::Value::Dict optional_value = json_value->GetDict().Clone();
  optional_value.Remove("logo");
  optional_value.Remove("token_id");
  optional_value.Remove("coingecko_id");
  expected_token->logo = "";
  token = ValueToBlockchainToken(optional_value, "0x1", mojom::CoinType::ETH);
  EXPECT_EQ(token, expected_token);
}

TEST(ValueConversionUtilsUnitTest, PermissionRequestResponseToValue) {
  url::Origin origin = url::Origin::Create(GURL("https://brave.com"));
  std::vector<std::string> accounts{
      "0xA99D71De40D67394eBe68e4D0265cA6C9D421029"};
  base::Value::List value = PermissionRequestResponseToValue(origin, accounts);

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

  ASSERT_EQ(value.size(), 1UL);

  auto& param0 = value[0].GetDict();
  auto* caveats = param0.FindList("caveats");
  ASSERT_NE(caveats, nullptr);
  ASSERT_EQ(caveats->size(), 2UL);

  auto& caveats0 = (*caveats)[0].GetDict();
  std::string* name = caveats0.FindString("name");
  ASSERT_NE(name, nullptr);
  EXPECT_EQ(*name, "primaryAccountOnly");
  std::string* type = caveats0.FindString("type");
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "limitResponseLength");
  absl::optional<int> primary_accounts_only_value = caveats0.FindInt("value");
  ASSERT_NE(primary_accounts_only_value, absl::nullopt);
  EXPECT_EQ(*primary_accounts_only_value, 1);

  auto& caveats1 = (*caveats)[1].GetDict();
  name = caveats1.FindString("name");
  ASSERT_NE(name, nullptr);
  EXPECT_EQ(*name, "exposedAccounts");
  type = caveats1.FindString("type");
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(*type, "filterResponse");
  auto* exposed_accounts = caveats1.FindList("value");
  ASSERT_NE(exposed_accounts, nullptr);
  ASSERT_EQ(exposed_accounts->size(), 1UL);
  EXPECT_EQ((*exposed_accounts)[0],
            base::Value("0xA99D71De40D67394eBe68e4D0265cA6C9D421029"));

  auto* context = param0.FindList("context");
  ASSERT_NE(context, nullptr);
  ASSERT_EQ(context->size(), 1UL);
  EXPECT_EQ((*context)[0], base::Value("https://github.com/MetaMask/rpc-cap"));

  absl::optional<double> date = param0.FindDouble("date");
  ASSERT_NE(date, absl::nullopt);

  std::string* id = param0.FindString("id");
  ASSERT_NE(id, nullptr);

  std::string* invoker = param0.FindString("invoker");
  ASSERT_NE(invoker, nullptr);
  EXPECT_EQ(*invoker, "https://brave.com");

  std::string* parent_capability = param0.FindString("parentCapability");
  ASSERT_NE(parent_capability, nullptr);
  EXPECT_EQ(*parent_capability, "eth_accounts");
}

}  // namespace brave_wallet
