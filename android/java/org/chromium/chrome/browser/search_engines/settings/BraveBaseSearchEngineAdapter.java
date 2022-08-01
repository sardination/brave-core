/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.search_engines.settings;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

import androidx.annotation.StringRes;

import org.chromium.chrome.browser.search_engines.TemplateUrlServiceFactory;
import org.chromium.chrome.browser.search_engines.settings.SearchEngineAdapter;
import org.chromium.components.search_engines.TemplateUrl;
import org.chromium.components.search_engines.TemplateUrlService;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

public class BraveBaseSearchEngineAdapter extends BaseAdapter {
    public BraveBaseSearchEngineAdapter() {
    }

    // BaseAdapter:
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	return convertView;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public Object getItem(int pos) {
        return null;
    }

    @Override
    public int getCount() {
        return 0;
    }

    public static void sortAndFilterUnnecessaryTemplateUrl(
            List<TemplateUrl> templateUrls, TemplateUrl defaultSearchEngine) {
        int recentEngineNum = 0;
        long displayTime = System.currentTimeMillis() - SearchEngineAdapter.MAX_DISPLAY_TIME_SPAN_MS;
        Set<String> templateUrlSet = new HashSet<String>();
        Iterator<TemplateUrl> iterator = templateUrls.iterator();
        while (iterator.hasNext()) {
            TemplateUrl templateUrl = iterator.next();
            if (!templateUrlSet.contains(templateUrl.getShortName())) {
                templateUrlSet.add(templateUrl.getShortName());
            } else {
                iterator.remove();
                continue;
            }
            if (getSearchEngineSourceType(templateUrl, defaultSearchEngine)
                    != SearchEngineAdapter.TemplateUrlSourceType.RECENT) {
                continue;
            }
            if (recentEngineNum < SearchEngineAdapter.MAX_RECENT_ENGINE_NUM
                    && templateUrl.getLastVisitedTime() > displayTime) {
                recentEngineNum++;
            } else {
                iterator.remove();
            }
        }
    }

    public static @SearchEngineAdapter.TemplateUrlSourceType int getSearchEngineSourceType(
            TemplateUrl templateUrl, TemplateUrl defaultSearchEngine) {
        if (templateUrl.getIsPrepopulated()) {
            return SearchEngineAdapter.TemplateUrlSourceType.PREPOPULATED;
        } else if (templateUrl.equals(defaultSearchEngine)) {
            return SearchEngineAdapter.TemplateUrlSourceType.DEFAULT;
        } else {
            return SearchEngineAdapter.TemplateUrlSourceType.RECENT;
        }
    }

    // Fields and empty methods below are to satisfy java compiler and will be
    // removed by bytecode asm
    public List<TemplateUrl> mPrepopulatedSearchEngines = new ArrayList<>();
    public List<TemplateUrl> mRecentSearchEngines = new ArrayList<>();
    public int mSelectedSearchEnginePosition = -1;

    public static boolean containsTemplateUrl(
            List<TemplateUrl> templateUrls, TemplateUrl targetTemplateUrl) {
        assert false;
        return true;
    }

    public int computeStartIndexForRecentSearchEngines() {
        assert false;
        return -1;
    }

    public boolean didSearchEnginesChange(List<TemplateUrl> templateUrls) {
        if (templateUrls.size()
                != mPrepopulatedSearchEngines.size() + mRecentSearchEngines.size()) {
            return true;
        }
        for (int i = 0; i < templateUrls.size(); i++) {
            TemplateUrl templateUrl = templateUrls.get(i);
            if (!containsTemplateUrl(mPrepopulatedSearchEngines, templateUrl)
                    && !containsTemplateUrl(mRecentSearchEngines, templateUrl)) {
                return true;
            }
        }
        // Till this line it is a straight copy of SearchEngineAdapter.didSearchEnginesChange.
        // The original method does not give true when the set of engines wasn't
        // changed but the selected default search engine - was changed. This
        // happened because Chromium does not sync the DSE.
        // The code below is in fact part of SearchEngineAdapter.refreshData
        // which detects new mSelectedSearchEnginePosition

        TemplateUrlService templateUrlService = TemplateUrlServiceFactory.get();
        assert templateUrlService.isLoaded();

        TemplateUrl defaultSearchEngineTemplateUrl =
                templateUrlService.getDefaultSearchEngineTemplateUrl();

        if (mSelectedSearchEnginePosition == -1) {
            return false;
        }

        // Convert the TemplateUrl index into an index of mSearchEngines.
        int selectedSearchEnginePosition = -1;
        for (int i = 0; i < mPrepopulatedSearchEngines.size(); ++i) {
            if (mPrepopulatedSearchEngines.get(i).equals(defaultSearchEngineTemplateUrl)) {
                selectedSearchEnginePosition = i;
            }
        }

        for (int i = 0; i < mRecentSearchEngines.size(); ++i) {
            if (mRecentSearchEngines.get(i).equals(defaultSearchEngineTemplateUrl)) {
                // Add one to offset the title for the recent search engine list.
                selectedSearchEnginePosition = i + computeStartIndexForRecentSearchEngines();
            }
        }

        return selectedSearchEnginePosition != mSelectedSearchEnginePosition;
    }
}
