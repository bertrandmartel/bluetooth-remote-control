package com.github.akinaru.bleremote.inter;

import android.view.View;

import com.github.akinaru.bleremote.model.BitmapObj;

/**
 * Created by akinaru on 11/07/16.
 */
public interface IViewHolderLongClickListener {

    void onClick(View v, BitmapObj bm, boolean remove);

}
