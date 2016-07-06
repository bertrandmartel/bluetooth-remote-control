package com.github.akinaru.bledisplayremote.inter;

import com.github.akinaru.bledisplayremote.model.Button;
import com.github.akinaru.bledisplayremote.model.ButtonState;

/**
 * Created by akinaru on 13/04/16.
 */
public interface IButtonListener {

    void onButtonStateChange(Button button, ButtonState state);
}
