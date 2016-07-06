package com.github.akinaru.bledisplayremote.inter;

import com.github.akinaru.bledisplayremote.model.DpadState;

/**
 * Created by akinaru on 13/04/16.
 */
public interface IDirectionPadListener {

    void onDPadStateChanged(DpadState state);
}
