package com.github.akinaru.bleremote.inter;

import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;

/**
 * Created by akinaru on 13/04/16.
 */
public interface IBleDisplayRemoteDevice {

    void addButtonListener(IButtonListener listener);

    void pushLedState(byte mask, IPushListener listener);

    void addDirectionPadListener(IDirectionPadListener listener);

    void pushFullColor(byte red, byte green, byte blue, IPushListener listener);

    void sendBitmapNoEncoding(final byte[] bitmapData);

    void sendBitmapEncodedBitmask(final byte[] bitmapData);
    
}
