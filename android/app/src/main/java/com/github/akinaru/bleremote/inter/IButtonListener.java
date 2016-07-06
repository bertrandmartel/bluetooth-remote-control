package com.github.akinaru.bleremote.inter;

import com.github.akinaru.bleremote.model.Button;
import com.github.akinaru.bleremote.model.ButtonState;

/**
 * Created by akinaru on 13/04/16.
 */
public interface IButtonListener {

    void onButtonStateChange(Button button, ButtonState state);
}
