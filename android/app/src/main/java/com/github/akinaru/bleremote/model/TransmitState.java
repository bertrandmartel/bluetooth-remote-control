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
public enum TransmitState {

    TRANSMIT_NONE(0),
    TRANSMITTING(1),
    TRANSMIT_OK(2),
    TRANSMIT_COMPLETE(3),
    TRANSMIT_ERROR(4),
    TRANSMIT_CANCEL(5),
    TRANSMIT_START(6);

    private int state;

    TransmitState(int state) {
        this.state = state;
    }

    public static TransmitState getTransmitState(byte data) {

        switch (data) {
            case 0:
                return TransmitState.TRANSMIT_NONE;
            case 1:
                return TransmitState.TRANSMITTING;
            case 2:
                return TransmitState.TRANSMIT_OK;
            case 3:
                return TransmitState.TRANSMIT_COMPLETE;
            case 4:
                return TransmitState.TRANSMIT_ERROR;
            case 5:
                return TransmitState.TRANSMIT_CANCEL;
            case 6:
                return TransmitState.TRANSMIT_START;
        }
        return TransmitState.TRANSMIT_NONE;
    }
}