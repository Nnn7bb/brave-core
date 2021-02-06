/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/browser/ads_tab_helper.h"

#include <memory>
#include <utility>

#include "base/logging.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/browser/ads_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "components/dom_distiller/content/browser/distiller_javascript_utils.h"
#include "components/dom_distiller/content/browser/distiller_page_web_contents.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/page_transition_types.h"
#include "ui/base/resource/resource_bundle.h"

#if !defined(OS_ANDROID)
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#endif

namespace brave_ads {

namespace {

void DebugLog(content::NavigationHandle* navigation_handle) {
  VLOG(0) << "FOOBAR.has_user_gesture: " << navigation_handle->HasUserGesture();

  VLOG(0) << "FOOBAR.was_initiated_by_link_click: "
          << navigation_handle->WasInitiatedByLinkClick();

  VLOG(0) << "FOOBAR.is_download: " << navigation_handle->IsDownload();

  const ui::PageTransition page_transition =
      navigation_handle->GetPageTransition();

  std::string page_transition_as_string;
  switch (ui::PageTransitionStripQualifier(page_transition)) {
    case ui::PAGE_TRANSITION_LINK: {
      page_transition_as_string = "Link";
      break;
    }

    case ui::PAGE_TRANSITION_TYPED: {
      page_transition_as_string = "Typed";
      break;
    }

    case ui::PAGE_TRANSITION_AUTO_BOOKMARK: {
      page_transition_as_string = "Auto Bookmark";
      break;
    }

    case ui::PAGE_TRANSITION_AUTO_SUBFRAME: {
      page_transition_as_string = "Auto Subframe";
      break;
    }

    case ui::PAGE_TRANSITION_MANUAL_SUBFRAME: {
      page_transition_as_string = "Manual Subframe";
      break;
    }

    case ui::PAGE_TRANSITION_GENERATED: {
      page_transition_as_string = "Generated";
      break;
    }

    case ui::PAGE_TRANSITION_AUTO_TOPLEVEL: {
      page_transition_as_string = "Auto Top Level";
      break;
    }

    case ui::PAGE_TRANSITION_FORM_SUBMIT: {
      page_transition_as_string = "Form Submit";
      break;
    }

    case ui::PAGE_TRANSITION_RELOAD: {
      page_transition_as_string = "Reload";
      break;
    }

    case ui::PAGE_TRANSITION_KEYWORD: {
      page_transition_as_string = "Keyword";
      break;
    }

    case ui::PAGE_TRANSITION_KEYWORD_GENERATED: {
      page_transition_as_string = "Keyword Generated";
      break;
    }

    case ui::PAGE_TRANSITION_FORWARD_BACK: {
      page_transition_as_string = "Forward/Back";
      break;
    }

    case ui::PAGE_TRANSITION_FROM_ADDRESS_BAR: {
      page_transition_as_string = "From Address Bar";
      break;
    }

    case ui::PAGE_TRANSITION_HOME_PAGE: {
      page_transition_as_string = "Home Page";
      break;
    }

    case ui::PAGE_TRANSITION_FROM_API: {
      page_transition_as_string = "From API";
      break;
    }

    case ui::PAGE_TRANSITION_CHAIN_START: {
      page_transition_as_string = "Chain Start";
      break;
    }

    case ui::PAGE_TRANSITION_CHAIN_END: {
      page_transition_as_string = "Chain End";
      break;
    }

    case ui::PAGE_TRANSITION_CLIENT_REDIRECT: {
      page_transition_as_string = "Client Redirect";
      break;
    }

    case ui::PAGE_TRANSITION_SERVER_REDIRECT: {
      page_transition_as_string = "Server Redirect";
      break;
    }

    default: {
      page_transition_as_string = "Other";
      break;
    }
  }

  VLOG(0) << "FOOBAR.transition: " << page_transition_as_string;
}

}  // namespace

AdsTabHelper::AdsTabHelper(content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      tab_id_(sessions::SessionTabHelper::IdForTab(web_contents)),
      ads_service_(nullptr),
      is_active_(false),
      is_browser_active_(true),
      should_process_(false),
      weak_factory_(this) {
  if (!tab_id_.is_valid()) {
    return;
  }

  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  ads_service_ = AdsServiceFactory::GetForProfile(profile);

#if !defined(OS_ANDROID)
  BrowserList::AddObserver(this);
  OnBrowserSetLastActive(BrowserList::GetInstance()->GetLastActive());
#endif
  OnVisibilityChanged(web_contents->GetVisibility());
}

AdsTabHelper::~AdsTabHelper() {
#if !defined(OS_ANDROID)
  BrowserList::RemoveObserver(this);
#endif
}

bool AdsTabHelper::IsAdsEnabled() const {
  if (!ads_service_ || !ads_service_->IsEnabled()) {
    return false;
  }

  return true;
}

void AdsTabHelper::TabUpdated() {
  if (!IsAdsEnabled()) {
    return;
  }

  ads_service_->OnTabUpdated(tab_id_, web_contents()->GetURL(), is_active_,
                             is_browser_active_);
}

void AdsTabHelper::RunIsolatedJavaScript(
    content::RenderFrameHost* render_frame_host) {
  DCHECK(render_frame_host);

  dom_distiller::RunIsolatedJavaScript(
      render_frame_host, "document.body.innerText",
      base::BindOnce(&AdsTabHelper::OnJavaScriptResult,
                     weak_factory_.GetWeakPtr()));
}

void AdsTabHelper::OnJavaScriptResult(base::Value value) {
  if (!IsAdsEnabled()) {
    return;
  }

  DCHECK(value.is_string());
  std::string content;
  value.GetAsString(&content);

  VLOG(0) << "FOOBAR.redirect_chain:";
  for (const auto& redirect : redirect_chain_) {
    VLOG(0) << "  " << redirect.spec();
  }

  ads_service_->OnPageLoaded(tab_id_, page_transition_, has_user_gesture_,
                             redirect_chain_, content);
}

void AdsTabHelper::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  DCHECK(navigation_handle);

  if (!IsAdsEnabled() || !navigation_handle->IsInMainFrame() ||
      !navigation_handle->HasCommitted() || !tab_id_.is_valid()) {
    return;
  }

  redirect_chain_ = navigation_handle->GetRedirectChain();

  const ui::PageTransition page_transition =
      navigation_handle->GetPageTransition();

  DebugLog(navigation_handle);

  page_transition_ = static_cast<int32_t>(page_transition);

  has_user_gesture_ = navigation_handle->HasUserGesture();

  if (!navigation_handle->IsSameDocument()) {
    should_process_ = navigation_handle->GetRestoreType() ==
                      content::RestoreType::kNotRestored;
    return;
  }

  content::RenderFrameHost* render_frame_host =
      navigation_handle->GetRenderFrameHost();

  RunIsolatedJavaScript(render_frame_host);
}

void AdsTabHelper::DocumentOnLoadCompletedInMainFrame() {
  if (!IsAdsEnabled() || !should_process_) {
    return;
  }

  std::unique_ptr<dom_distiller::SourcePageHandleWebContents> handle =
      std::make_unique<dom_distiller::SourcePageHandleWebContents>(
          web_contents(), false);

  content::RenderFrameHost* render_frame_host =
      handle->web_contents()->GetMainFrame();

  RunIsolatedJavaScript(render_frame_host);
}

void AdsTabHelper::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                                 const GURL& validated_url) {
  DCHECK(render_frame_host);

  if (render_frame_host->GetParent()) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::MediaStartedPlaying(const MediaPlayerInfo& video_type,
                                       const content::MediaPlayerId& id) {
  if (!IsAdsEnabled()) {
    return;
  }

  ads_service_->OnMediaStart(tab_id_);
}

void AdsTabHelper::MediaStoppedPlaying(
    const MediaPlayerInfo& video_type,
    const content::MediaPlayerId& id,
    WebContentsObserver::MediaStoppedReason reason) {
  if (!IsAdsEnabled()) {
    return;
  }

  ads_service_->OnMediaStop(tab_id_);
}

void AdsTabHelper::OnVisibilityChanged(content::Visibility visibility) {
  const bool old_is_active = is_active_;

  switch (visibility) {
    case content::Visibility::HIDDEN:
    case content::Visibility::OCCLUDED: {
      is_active_ = false;
      break;
    }

    case content::Visibility::VISIBLE: {
      is_active_ = true;
      break;
    }
  }

  if (old_is_active == is_active_) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::WebContentsDestroyed() {
  if (!IsAdsEnabled()) {
    return;
  }

  ads_service_->OnTabClosed(tab_id_);
  ads_service_ = nullptr;
}

#if !defined(OS_ANDROID)
// components/brave_ads/browser/background_helper_android.cc handles Android
void AdsTabHelper::OnBrowserSetLastActive(Browser* browser) {
  if (!browser) {
    return;
  }

  const bool old_is_browser_active = is_browser_active_;

  if (browser->tab_strip_model()->GetIndexOfWebContents(web_contents()) !=
      TabStripModel::kNoTab) {
    is_browser_active_ = true;
  }

  if (old_is_browser_active == is_browser_active_) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::OnBrowserNoLongerActive(Browser* browser) {
  DCHECK(browser);

  const bool old_is_browser_active = is_browser_active_;

  if (browser->tab_strip_model()->GetIndexOfWebContents(web_contents()) !=
      TabStripModel::kNoTab) {
    is_browser_active_ = false;
  }

  if (old_is_browser_active == is_browser_active_) {
    return;
  }

  TabUpdated();
}
#endif

WEB_CONTENTS_USER_DATA_KEY_IMPL(AdsTabHelper)

}  // namespace brave_ads
