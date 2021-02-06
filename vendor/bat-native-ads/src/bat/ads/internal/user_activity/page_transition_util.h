/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_PAGE_TRANSITION_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_PAGE_TRANSITION_UTIL_H_

#include <stdint.h>

#include <string>

#include "base/optional.h"
#include "bat/ads/internal/user_activity/user_activity_event_types.h"

namespace ads {

bool IsNewNavigation(const int32_t page_transition);

bool DidUseBackOrFowardButtonToTriggerNavigation(const int32_t page_transition);

bool DidUseAddressBarToTriggerNavigation(const int32_t page_transition);

bool DidNavigateToHomePage(const int32_t page_transition);

bool DidTransitionFromExternalApplication(const int32_t page_transition);

std::string GetCoreTransitionStringFromInt(const int32_t page_transition);

int32_t GetQualifierFromInt(const int32_t page_transition);

base::Optional<UserActivityEventType> ToUserActivityEventType(
    const int32_t page_transition);

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_PAGE_TRANSITION_UTIL_H_
