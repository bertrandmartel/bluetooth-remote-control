/********************************************************************************
 * The MIT License (MIT)                                                        *
 * <p/>                                                                         *
 * Copyright (c) 2016 Bertrand Martel                                           *
 * <p/>                                                                         *
 * Permission is hereby granted, free of charge, to any person obtaining a copy *
 * of this software and associated documentation files (the "Software"), to deal*
 * in the Software without restriction, including without limitation the rights *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    *
 * copies of the Software, and to permit persons to whom the Software is        *
 * furnished to do so, subject to the following conditions:                     *
 * <p/>                                                                         *
 * The above copyright notice and this permission notice shall be included in   *
 * all copies or substantial portions of the Software.                          *
 * <p/>                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,*
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    *
 * THE SOFTWARE.                                                                *
 */

package com.github.akinaru.bleremote.menu;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.v4.widget.DrawerLayout;
import android.view.MenuItem;

import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.dialog.AboutDialog;
import com.github.akinaru.bleremote.dialog.OpenSourceItemsDialog;
import com.github.akinaru.bleremote.inter.IRemoteActivity;

/**
 * Some functions used to manage Menu
 *
 * @author Bertrand Martel
 */
public class MenuUtils {

    /**
     * Execute actions according to selected menu item
     *
     * @param menuItem MenuItem object
     * @param mDrawer  navigation drawer
     * @param context  android context
     */
    public static void selectDrawerItem(MenuItem menuItem, DrawerLayout mDrawer, Context context, IRemoteActivity remoteActivity) {

        switch (menuItem.getItemId()) {
            case R.id.disconnect_device: {
                if (remoteActivity != null) {
                    remoteActivity.disconnect();
                }
                break;
            }
            case R.id.report_bugs: {
                Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts(
                        "mailto", context.getResources().getString(R.string.email_addr), null));
                intent.putExtra(Intent.EXTRA_SUBJECT, context.getResources().getString(R.string.issue_subject));
                intent.putExtra(Intent.EXTRA_TEXT, context.getResources().getString(R.string.report_hint));
                context.startActivity(Intent.createChooser(intent, context.getResources().getString(R.string.issue_title)));
                break;
            }
            case R.id.open_source_components: {
                OpenSourceItemsDialog d = new OpenSourceItemsDialog(context);
                d.show();
                break;
            }
            case R.id.about_app: {
                AboutDialog dialog = new AboutDialog(context);
                dialog.show();
                break;
            }
        }
        mDrawer.closeDrawers();
    }
}
