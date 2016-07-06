package com.github.akinaru.bledisplayremote.dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

import com.github.akinaru.bledisplayremote.R;

public class AboutDialog extends AlertDialog {

    public AboutDialog(Context context) {
        super(context);

        LayoutInflater inflater = getLayoutInflater();
        View dialoglayout = inflater.inflate(R.layout.about_dialog, null);
        setView(dialoglayout);

        TextView copyright = (TextView) dialoglayout.findViewById(R.id.copyright);
        TextView github_link = (TextView) dialoglayout.findViewById(R.id.github_link);

        copyright.setText(R.string.copyright);
        github_link.setText(R.string.github_link);

        setTitle(R.string.about);
        setButton(DialogInterface.BUTTON_POSITIVE, "Ok",
                (OnClickListener) null);
    }
}