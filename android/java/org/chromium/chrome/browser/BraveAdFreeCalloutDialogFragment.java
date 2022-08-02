/**
 * Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser;

import android.content.DialogInterface;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.fragment.app.DialogFragment;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.util.ConfigurationUtils;
import org.chromium.chrome.browser.util.TabUtils;
import org.chromium.ui.base.DeviceFormFactor;

public class BraveAdFreeCalloutDialogFragment extends DialogFragment {
    private ImageView mConfettiImageView;
    private final String YOUTUBE_SITE = "https://www.youtube.com";

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        setDialogParams();
    }

    @Override
    public void onResume() {
        super.onResume();
        getDialog().setOnKeyListener(new DialogInterface.OnKeyListener() {
            @Override
            public boolean onKey(android.content.DialogInterface dialog, int keyCode,
                    android.view.KeyEvent event) {
                if ((keyCode == android.view.KeyEvent.KEYCODE_BACK)) {
                    dismiss();
                    return true;
                } else
                    return false;
            }
        });
        setDialogParams();
    }

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_ad_free_callout_dialog, container, false);

        if (getDialog() != null && getDialog().getWindow() != null) {
            getDialog().getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        }

        return view;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        mConfettiImageView = view.findViewById(R.id.confetti_imageview);

        Button youtubeButton = view.findViewById(R.id.btn_youtube);
        youtubeButton.setOnClickListener(button -> {
            TabUtils.openUrlInSameTab(YOUTUBE_SITE);
            /*if (((BraveActivity) getActivity()) != null) {
                ((BraveActivity) getActivity())
                        .openNewOrSelectExistingTab(BraveConstants.YOUTUBE_LINK);
            }*/
            dismiss();
        });

        ImageView cancelImageView = view.findViewById(R.id.cancel_imageview);
        cancelImageView.setOnClickListener(button -> { dismiss(); });
    }

    private void setDialogParams() {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getActivity().getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int mDeviceHeight = displayMetrics.heightPixels;
        int mDeviceWidth = displayMetrics.widthPixels;

        WindowManager.LayoutParams params = getDialog().getWindow().getAttributes();
        boolean isTablet = DeviceFormFactor.isNonMultiDisplayContextOnTablet(getActivity());
        boolean isLandscape = ConfigurationUtils.isLandscape(getActivity());
        if (isTablet || isLandscape) {
            params.width = (int) (0.5 * mDeviceWidth);
        } else {
            params.width = (int) (0.9 * mDeviceWidth);
        }

        if (mConfettiImageView != null) {
            if (isLandscape && !isTablet) {
                mConfettiImageView.setVisibility(View.GONE);
            } else {
                mConfettiImageView.setVisibility(View.VISIBLE);
            }
        }
        params.height = LinearLayout.LayoutParams.WRAP_CONTENT;
        getDialog().getWindow().setAttributes(params);
    }
}
