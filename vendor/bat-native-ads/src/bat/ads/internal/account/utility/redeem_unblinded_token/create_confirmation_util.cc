/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/account/utility/redeem_unblinded_token/create_confirmation_util.h"

#include <utility>

#include "base/base64url.h"
#include "base/check.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/notreached.h"
#include "base/values.h"
#include "bat/ads/internal/account/confirmations/confirmation_info.h"
#include "bat/ads/internal/privacy/challenge_bypass_ristretto/token_preimage.h"
#include "bat/ads/internal/privacy/challenge_bypass_ristretto/verification_key.h"
#include "bat/ads/internal/privacy/challenge_bypass_ristretto/verification_signature.h"
#include "bat/ads/internal/privacy/tokens/unblinded_tokens/unblinded_token_info.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {

std::string CreateConfirmationRequestDTO(const ConfirmationInfo& confirmation) {
  base::Value::Dict dto;

  dto.Set("creativeInstanceId", confirmation.creative_instance_id);

  dto.Set("transactionId", confirmation.transaction_id);

  dto.Set("payload", base::Value::Dict());

  base::Value::List blinded_payment_tokens;
  const absl::optional<std::string> blinded_payment_token_base64_optional =
      confirmation.blinded_payment_token.EncodeBase64();
  if (blinded_payment_token_base64_optional) {
    blinded_payment_tokens.Append(
        blinded_payment_token_base64_optional.value());
  }
  dto.Set("blindedPaymentTokens", std::move(blinded_payment_tokens));

  dto.Set("type", confirmation.type.ToString());

  const absl::optional<std::string> public_key_base64_optional =
      confirmation.unblinded_token.public_key.EncodeBase64();
  if (public_key_base64_optional) {
    dto.Set("publicKey", public_key_base64_optional.value());
  }

  absl::optional<base::Value> user_data =
      base::JSONReader::Read(confirmation.user_data);
  if (user_data && user_data->is_dict()) {
    dto.Merge(std::move(user_data->GetDict()));
  }

  std::string json;
  base::JSONWriter::Write(dto, &json);

  return json;
}

std::string CreateCredential(const privacy::UnblindedTokenInfo& unblinded_token,
                             const std::string& payload) {
  DCHECK(!payload.empty());

  const absl::optional<privacy::cbr::VerificationKey>
      verification_key_optional = unblinded_token.value.DeriveVerificationKey();
  if (!verification_key_optional) {
    NOTREACHED();
    return "";
  }
  privacy::cbr::VerificationKey verification_key =
      verification_key_optional.value();

  const absl::optional<privacy::cbr::VerificationSignature>
      verification_signature_optional = verification_key.Sign(payload);
  if (!verification_signature_optional) {
    NOTREACHED();
    return "";
  }
  const privacy::cbr::VerificationSignature& verification_signature =
      verification_signature_optional.value();

  const absl::optional<std::string> verification_signature_base64_optional =
      verification_signature.EncodeBase64();
  if (!verification_signature_base64_optional) {
    NOTREACHED();
    return "";
  }

  const absl::optional<privacy::cbr::TokenPreimage> token_preimage_optional =
      unblinded_token.value.GetTokenPreimage();
  if (!token_preimage_optional) {
    NOTREACHED();
    return "";
  }
  const privacy::cbr::TokenPreimage& token_preimage =
      token_preimage_optional.value();

  const absl::optional<std::string> token_preimage_base64_optional =
      token_preimage.EncodeBase64();
  if (!token_preimage_base64_optional) {
    NOTREACHED();
    return "";
  }

  base::Value::Dict dict;

  dict.Set("payload", payload);
  dict.Set("signature", verification_signature_base64_optional.value());
  dict.Set("t", token_preimage_base64_optional.value());

  std::string json;
  base::JSONWriter::Write(dict, &json);

  std::string credential_base64url;
  base::Base64UrlEncode(json, base::Base64UrlEncodePolicy::INCLUDE_PADDING,
                        &credential_base64url);

  return credential_base64url;
}

}  // namespace ads
