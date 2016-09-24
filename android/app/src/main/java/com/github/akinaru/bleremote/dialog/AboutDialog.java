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

package com.github.akinaru.bleremote.dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

import com.github.akinaru.bleremote.BuildConfig;
import com.github.akinaru.bleremote.R;

/**
 * About dialog
 *
 * @author Bertrand Martel
 */
public class AboutDialog extends AlertDialog {

    public AboutDialog(Context context) {
        super(context);

        LayoutInflater inflater = getLayoutInflater();
        View dialoglayout = inflater.inflate(R.layout.about_dialog, null);
        setView(dialoglayout);

        TextView name = (TextView) dialoglayout.findViewById(R.id.name);
        TextView copyright = (TextView) dialoglayout.findViewById(R.id.copyright);
        TextView github_link = (TextView) dialoglayout.findViewById(R.id.github_link);

        name.setText(context.getResources().getString(R.string.app_name) + " v" + BuildConfig.VERSION_NAME);
        copyright.setText(R.string.copyright);
        github_link.setText(R.string.github_link);

        setTitle(R.string.about);
        setButton(DialogInterface.BUTTON_POSITIVE, context.getResources().getString(R.string.dialog_ok),
                (OnClickListener) null);
    }
}