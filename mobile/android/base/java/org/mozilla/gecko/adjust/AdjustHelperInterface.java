/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.adjust;

import android.content.Context;
import android.content.Intent;

public interface AdjustHelperInterface {
    /**
     * Register the Application with the Adjust SDK.
     * @param appToken the (secret!) Adjust SDK per-application token to register with; may be null.
     */
    void onCreate(final Context context, final String appToken, final AttributionHelperListener... listener);
    void onPause();
    void onResume();

    void setEnabled(final boolean isEnabled);
    void onReceive(final Context context, final Intent intent);
}
