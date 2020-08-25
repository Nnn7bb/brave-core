/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoint/uphold/get_card/get_card.h"

#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/endpoint/uphold/uphold_utils.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/uphold/uphold_card.h"
#include "net/http/http_status_code.h"

using std::placeholders::_1;

namespace ledger {
namespace endpoint {
namespace uphold {

GetCard::GetCard(bat_ledger::LedgerImpl* ledger):
    ledger_(ledger) {
  DCHECK(ledger_);
}

GetCard::~GetCard() = default;

std::string GetCard::GetUrl(const std::string& address) {
  return GetServerUrl("/v0/me/cards/" + address);
}

ledger::Result GetCard::CheckStatusCode(const int status_code) {
  if (status_code == net::HTTP_UNAUTHORIZED ||
      status_code == net::HTTP_NOT_FOUND ||
      status_code == net::HTTP_FORBIDDEN) {
    return ledger::Result::EXPIRED_TOKEN;
  }

  if (status_code != net::HTTP_OK) {
    return ledger::Result::LEDGER_ERROR;
  }

  return ledger::Result::LEDGER_OK;
}

ledger::Result GetCard::ParseBody(
    const std::string& body,
    double* available) {
  DCHECK(available);

  base::Optional<base::Value> value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid JSON");
    return ledger::Result::LEDGER_ERROR;
  }

  base::DictionaryValue* dictionary = nullptr;
  if (!value->GetAsDictionary(&dictionary)) {
    BLOG(0, "Invalid JSON");
    return ledger::Result::LEDGER_ERROR;
  }

  const auto* available_str = dictionary->FindStringKey("available");
  if (!available_str) {
    BLOG(0, "Missing available");
    return ledger::Result::LEDGER_ERROR;
  }

  const bool success = base::StringToDouble(*available_str, available);
  if (!success) {
    *available = 0.0;
  }

  return ledger::Result::LEDGER_OK;
}

void GetCard::Request(
    const std::string& address,
    const std::string& token,
    GetCardCallback callback) {
  auto url_callback = std::bind(&GetCard::OnRequest,
      this,
      _1,
      callback);
  ledger_->LoadURL(
      GetUrl(address),
      RequestAuthorization(token),
      "",
      "",
      ledger::UrlMethod::GET,
      url_callback);
}

void GetCard::OnRequest(
    const ledger::UrlResponse& response,
    GetCardCallback callback) {
  ledger::LogUrlResponse(__func__, response);

  ledger::Result result = CheckStatusCode(response.status_code);

  if (result != ledger::Result::LEDGER_OK) {
    callback(result, 0.0);
    return;
  }

  double available;
  result = ParseBody(response.body, &available);
  callback(result, available);
}

}  // namespace uphold
}  // namespace endpoint
}  // namespace ledger
