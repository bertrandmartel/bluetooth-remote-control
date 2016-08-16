/********************************************************************************
 * The MIT License (MIT)                                                        *
 * <p/>                                                                         *
 * Copyright (c) 2016 Bertrand Martel                                           *
 * <p/>                                                                         *
 * Permission is hereby granted, free of charge, to any person obtaining a copy *
 * of this software and associated documentation files (the "Software"), to deal*
 * in the Software without restriction, including without limitation the rights *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    *
 * copies of the Software, and to permit persons to whom the Software is        *
 * furnished to do so, subject to the following conditions:                     *
 * <p/>                                                                         *
 * The above copyright notice and this permission notice shall be included in   *
 * all copies or substantial portions of the Software.                          *
 * <p/>                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,*
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    *
 * THE SOFTWARE.                                                                *
 */

package com.github.akinaru.bleremote.model;

/**
 * @author Bertrand Martel
 */
public enum DpadState {
    NONE,
    DOWN,
    RIGHT,
    SELECT,
    UP,
    LEFT,
    BUTTON_VOICE,
    BUTTON_HOME,
    BUTTON_BACK,
    BUTTON_PLAY_PAUSE;

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
            case 6:
                return DpadState.BUTTON_VOICE;
            case 7:
                return DpadState.BUTTON_HOME;
            case 8:
                return DpadState.BUTTON_BACK;
            case 9:
                return DpadState.BUTTON_PLAY_PAUSE;
        }
        return DpadState.NONE;
    }
}