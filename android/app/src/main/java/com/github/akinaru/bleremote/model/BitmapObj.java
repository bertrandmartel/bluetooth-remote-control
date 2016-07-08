package com.github.akinaru.bleremote.model;

import android.graphics.Bitmap;

/**
 * Created by akinaru on 08/07/16.
 */
public class BitmapObj {

    private Bitmap bitmap;

    private int ressourceId;

    public BitmapObj(Bitmap bitmap, int ressourceId) {
        this.bitmap = bitmap;
        this.ressourceId = ressourceId;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public int getRessourceId() {
        return ressourceId;
    }
}
