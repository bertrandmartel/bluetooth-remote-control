package com.github.akinaru.bleremote.inter;

/**
 * Created by akinaru on 26/07/16.
 */
public interface IProgressListener {

    void onProgress(int progress);

    void onFinishUpload();

    void onComplete();
}
