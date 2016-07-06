package com.github.akinaru.bledisplayremote.model;

/**
 * Created by akinaru on 19/04/16.
 */
public enum TransmitState {

    TRANSMIT_NONE(0),
    TRANSMITTING(1),
    TRANSMIT_OK(2),
    TRANSMIT_COMPLETE(3);

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
        }
        return TransmitState.TRANSMIT_NONE;
    }
}