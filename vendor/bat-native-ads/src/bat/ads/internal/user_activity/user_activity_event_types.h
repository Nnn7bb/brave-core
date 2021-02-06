/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_USER_ACTIVITY_EVENT_TYPES_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_USER_ACTIVITY_EVENT_TYPES_H_

#include <stdint.h>

namespace ads {

// WARNING: don't change these numbers. They are provided by the variations
// service, so future versions will need the same values to match the enums

enum class UserActivityEventType : int8_t {
  kBrowserWindowDidBecomeActive = 0,
  kBrowserWindowDidEnterBackground,
  kClickedBackOrForwardNavigationButtons,
  kClickedBookmark,
  kClickedHomePageButton,
  kClickedLink,
  kClickedReloadButton,
  kClosedTab,
  kFocusedOnExistingTab,
  kGeneratedKeyword,
  kNewNavigation,
  kOpenedLinkFromExternalApplication,
  kOpenedNewTab,
  kPlayedMedia,
  kStoppedPlayingMedia,
  kSubmittedForm,
  kTabUpdated,
  kTypedAndSelectedNonUrl,
  kTypedKeywordOtherThanDefaultSearchProvider,
  kTypedUrl,
  kUsedAddressBar
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_USER_ACTIVITY_USER_ACTIVITY_EVENT_TYPES_H_
