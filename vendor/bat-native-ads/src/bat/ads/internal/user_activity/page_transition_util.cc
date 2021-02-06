/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/user_activity/page_transition_util.h"

#include "ui/base/page_transition_types.h"

#include "bat/ads/internal/user_activity/user_activity.h"

namespace ads {

bool IsNewNavigation(const int32_t page_transition) {
  return ui::PageTransitionIsNewNavigation(
      ui::PageTransitionFromInt(page_transition));
}

bool DidUseBackOrFowardButtonToTriggerNavigation(
    const int32_t page_transition) {
  return (page_transition & ui::PAGE_TRANSITION_FORWARD_BACK) != 0;
}

bool DidUseAddressBarToTriggerNavigation(const int32_t page_transition) {
  return (page_transition & ui::PAGE_TRANSITION_FROM_ADDRESS_BAR) != 0;
}

bool DidNavigateToHomePage(const int32_t page_transition) {
  return (page_transition & ui::PAGE_TRANSITION_HOME_PAGE) != 0;
}

bool DidTransitionFromExternalApplication(const int32_t page_transition) {
  return (page_transition & ui::PAGE_TRANSITION_FROM_API) != 0;
}

std::string GetCoreTransitionStringFromInt(const int32_t page_transition) {
  return ui::PageTransitionGetCoreTransitionString(
      ui::PageTransitionFromInt(page_transition));
}

int32_t GetQualifierFromInt(const int32_t page_transition) {
  return ui::PageTransitionGetQualifier(
      ui::PageTransitionFromInt(page_transition));
}

base::Optional<UserActivityEventType> ToUserActivityEventType(
    const int32_t page_transition) {
  const ui::PageTransition ui_page_transition =
      ui::PageTransitionFromInt(page_transition);

  switch (ui::PageTransitionStripQualifier(ui_page_transition)) {
    case ui::PAGE_TRANSITION_LINK: {
      return UserActivityEventType::kClickedLink;
    }

    case ui::PAGE_TRANSITION_TYPED: {
      return UserActivityEventType::kTypedUrl;
    }

    case ui::PAGE_TRANSITION_AUTO_BOOKMARK: {
      return UserActivityEventType::kClickedBookmark;
    }

    case ui::PAGE_TRANSITION_GENERATED: {
      return UserActivityEventType::kTypedAndSelectedNonUrl;
    }

    case ui::PAGE_TRANSITION_FORM_SUBMIT: {
      return UserActivityEventType::kSubmittedForm;
    }

    case ui::PAGE_TRANSITION_RELOAD: {
      return UserActivityEventType::kClickedReloadButton;
    }

    case ui::PAGE_TRANSITION_KEYWORD: {
      return UserActivityEventType::kTypedKeywordOtherThanDefaultSearchProvider;
    }

    case ui::PAGE_TRANSITION_KEYWORD_GENERATED: {
      return UserActivityEventType::kGeneratedKeyword;
    }

    default: {
      return base::nullopt;
    }
  }
}

}  // namespace ads
