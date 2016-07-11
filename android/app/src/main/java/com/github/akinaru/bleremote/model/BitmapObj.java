package com.github.akinaru.bleremote.model;

import android.graphics.Bitmap;

/**
 * Created by akinaru on 08/07/16.
 */
public class BitmapObj {

    private Bitmap bitmap;

    private String fileName;

    public BitmapObj(Bitmap bitmap, String fileName) {
        this.fileName = fileName;
        this.bitmap = bitmap;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public String getFileName() {
        return fileName;
    }
}
