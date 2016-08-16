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
package com.github.akinaru.bleremote.bluetooth.connection;

import android.bluetooth.BluetoothGattCharacteristic;

import com.github.akinaru.bleremote.bluetooth.IDevice;
import com.github.akinaru.bleremote.bluetooth.listener.ICharacteristicListener;

/**
 * Bluetooth device implementation abstraction
 *
 * @author Bertrand Martel
 */
public abstract class BluetoothDeviceAbstr implements IDevice {


    private ICharacteristicListener characteristicListener;

    /**
     * bluetooth device gatt connection management
     */
    protected IBluetoothDeviceConn conn = null;

    /**
     * Give bluetooth device connection to device implementation object
     *
     * @param conn
     */
    public BluetoothDeviceAbstr(IBluetoothDeviceConn conn) {
        this.conn = conn;
    }


    /**
     * getter for bluetooth connection
     *
     * @return
     */
    public IBluetoothDeviceConn getConn() {
        return conn;
    }

    /**
     * notify characteristic read event
     *
     * @param characteristic Bluetooth characteristic
     */
    @Override
    public void notifyCharacteristicReadReceived(BluetoothGattCharacteristic characteristic) {
        characteristicListener.onCharacteristicReadReceived(characteristic);
    }

    @Override
    public void notifyCharacteristicWriteReceived(BluetoothGattCharacteristic characteristic) {
        characteristicListener.onCharacteristicWriteReceived(characteristic);
    }

    /**
     * notify characteritistic change event
     *
     * @param characteristic Bluetooth characteristic
     */
    @Override
    public void notifyCharacteristicChangeReceived(BluetoothGattCharacteristic characteristic) {
        characteristicListener.onCharacteristicChangeReceived(characteristic);
    }

    /**
     * getter for characteristic listener
     *
     * @return
     */
    public ICharacteristicListener getCharacteristicListener() {
        return characteristicListener;
    }

    /**
     * setter for characteristic listener
     *
     * @param listener
     */
    public void setCharacteristicListener(ICharacteristicListener listener) {
        characteristicListener = listener;
    }
}
