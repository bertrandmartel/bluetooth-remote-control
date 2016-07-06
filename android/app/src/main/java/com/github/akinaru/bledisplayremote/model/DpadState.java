package com.github.akinaru.bledisplayremote.model;

/**
 * Created by akinaru on 13/04/16.
 */
public enum DpadState {
    NONE,
    DOWN,
    RIGHT,
    SELECT,
    UP,
    LEFT;

    public static DpadState getDpad(byte data) {

        switch (data) {
            case 0:
                return DpadState.NONE;
            case 1:
                return DpadState.DOWN;
            case 2:
                return DpadState.RIGHT;
            case 3:
                return DpadState.SELECT;
            case 4:
                return DpadState.UP;
            case 5:
                return DpadState.LEFT;
        }
        return DpadState.NONE;
    }
}
