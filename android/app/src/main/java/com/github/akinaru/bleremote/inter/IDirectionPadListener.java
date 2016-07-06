package com.github.akinaru.bleremote.inter;

import com.github.akinaru.bleremote.model.DpadState;

/**
 * Created by akinaru on 13/04/16.
 */
public interface IDirectionPadListener {

    void onDPadStateChanged(DpadState state);
}
