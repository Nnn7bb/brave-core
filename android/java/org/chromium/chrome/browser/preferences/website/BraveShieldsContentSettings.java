/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.preferences.website;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.chrome.browser.preferences.website.BraveShieldsContentSettingsObserver;
import org.chromium.chrome.browser.profiles.Profile;

import java.util.ArrayList;
import java.util.List;

@JNINamespace("chrome::android")
public class BraveShieldsContentSettings {
    static public final String RESOURCE_IDENTIFIER_ADS = "ads";
    static public final String RESOURCE_IDENTIFIER_TRACKERS = "trackers";
    static public final String RESOURCE_IDENTIFIER_ADS_TRACKERS = "ads_trackers";
    static public final String RESOURCE_IDENTIFIER_HTTP_UPGRADABLE_RESOURCES = "httpUpgradableResources";
    static public final String RESOURCE_IDENTIFIER_BRAVE_SHIELDS = "braveShields";
    static public final String RESOURCE_IDENTIFIER_FINGERPRINTING = "fingerprinting";
    static public final String RESOURCE_IDENTIFIER_COOKIES = "cookies";
    static public final String RESOURCE_IDENTIFIER_REFERRERS = "referrers";
    static public final String RESOURCE_IDENTIFIER_JAVASCRIPTS = "javascript";

    static private final String blockResource = "block";
    static private final String blockThirdPartyResource = "block_third_party";
    static private final String allowResource = "allow";

    private long mNativeBraveShieldsContentSettings;
    private List<BraveShieldsContentSettingsObserver> mBraveShieldsContentSettingsObservers;
    private static BraveShieldsContentSettings sInstance;

    public static BraveShieldsContentSettings getInstance() {
        if (sInstance == null) sInstance = new BraveShieldsContentSettings();
        return sInstance;
    }

    private BraveShieldsContentSettings() {
        mNativeBraveShieldsContentSettings = 0;
        mBraveShieldsContentSettingsObservers =
                new ArrayList<BraveShieldsContentSettingsObserver>();
        init();
    }

    public void addObserver(BraveShieldsContentSettingsObserver observer) {
        mBraveShieldsContentSettingsObservers.add(observer);
    }

    public void removeObserver(BraveShieldsContentSettingsObserver observer) {
        mBraveShieldsContentSettingsObservers.remove(observer);
    }

    private void init() {
        if (mNativeBraveShieldsContentSettings == 0) {
            BraveShieldsContentSettingsJni.get().init(this);
        }
    }

    @Override
    protected void finalize() {
        destroy();
    }

    private void destroy() {
        if (mNativeBraveShieldsContentSettings == 0) {
            return;
        }
        BraveShieldsContentSettingsJni.get().destroy(mNativeBraveShieldsContentSettings);
    }

    static public void setShields(Profile profile, String host, String resourceIndentifier, boolean value,
            boolean fromTopShields) {
        String setting_string = (value ? blockResource : allowResource);
        if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_BRAVE_SHIELDS)) {
            BraveShieldsContentSettingsJni.get().setBraveShieldsEnabled(value, host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_ADS_TRACKERS)) {
            BraveShieldsContentSettingsJni.get().setAdControlType(setting_string, host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_HTTP_UPGRADABLE_RESOURCES)) {
            BraveShieldsContentSettingsJni.get().setHTTPSEverywhereEnabled(value, host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_COOKIES)) {
            if (setting_string.equals(blockResource)) {
                // On Android we block 3rd party cookies only
                setting_string = blockThirdPartyResource;
            }
            BraveShieldsContentSettingsJni.get().setCookieControlType(setting_string, host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_FINGERPRINTING)) {
            if (setting_string.equals(blockResource)) {
                // On Android we temporary control 3rd party fingerprint resources only,
                // until UI design for new shields menu is implemented
                setting_string = blockThirdPartyResource;
            }
            BraveShieldsContentSettingsJni.get().setFingerprintingControlType(setting_string, host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_JAVASCRIPTS)) {
            BraveShieldsContentSettingsJni.get().setNoScriptControlType(setting_string, host, profile);
        }
    }

    public static boolean getShields(Profile profile, String host, String resourceIndentifier) {
        String settings = blockResource;
        if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_BRAVE_SHIELDS)) {
            return BraveShieldsContentSettingsJni.get().getBraveShieldsEnabled(host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_ADS_TRACKERS)) {
            settings = BraveShieldsContentSettingsJni.get().getAdControlType(host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_HTTP_UPGRADABLE_RESOURCES)) {
            return BraveShieldsContentSettingsJni.get().getHTTPSEverywhereEnabled(host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_COOKIES)) {
            settings = BraveShieldsContentSettingsJni.get().getCookieControlType(host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_FINGERPRINTING)) {
            settings = BraveShieldsContentSettingsJni.get().getFingerprintingControlType(host, profile);
        } else if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_JAVASCRIPTS)) {
            settings = BraveShieldsContentSettingsJni.get().getNoScriptControlType(host, profile);
        }

        return !settings.equals(allowResource);
    }

    @CalledByNative
    private void setNativePtr(long nativePtr) {
        assert mNativeBraveShieldsContentSettings == 0;
        mNativeBraveShieldsContentSettings = nativePtr;
    }

    @CalledByNative
    private void blockedEvent(int tabId, String block_type, String subresource) {
        for (BraveShieldsContentSettingsObserver observer : mBraveShieldsContentSettingsObservers) {
            observer.blockEvent(tabId, block_type, subresource);
        }
    }

    @NativeMethods
    interface Natives {
        void init(BraveShieldsContentSettings self);
        void destroy(long nativeBraveShieldsContentSettings);

        void setBraveShieldsEnabled(boolean enabled, String url, Profile profile);
        boolean getBraveShieldsEnabled(String url, Profile profile);
        void setAdControlType(String type, String url, Profile profile);
        String getAdControlType(String url, Profile profile);
        void setCookieControlType(String type, String url, Profile profile);
        String getCookieControlType(String url, Profile profile);
        void setFingerprintingControlType(String type, String url, Profile profile);
        String getFingerprintingControlType(String url, Profile profile);
        void setHTTPSEverywhereEnabled(boolean enabled, String url, Profile profile);
        boolean getHTTPSEverywhereEnabled(String url, Profile profile);
        void setNoScriptControlType(String type, String url, Profile profile);
        String getNoScriptControlType(String url, Profile profile);
    }
}
