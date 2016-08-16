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
package com.github.akinaru.bleremote.bluetooth;

import android.bluetooth.BluetoothGattCharacteristic;

import com.github.akinaru.bleremote.bluetooth.listener.IDeviceInitListener;

/**
 * Generic template for bluetooth device
 *
 * @author Bertrand Martel
 */
public interface IDevice {

    /**
     * check if device is fully intitialized
     *
     * @return
     */
    boolean isInit();

    /**
     * add a device initialization listener
     *
     * @param listener
     */
    void addInitListener(IDeviceInitListener listener);

    /**
     * initialize device
     */
    void init();

    /**
     * callback called when result of characterisitic read is received
     *
     * @param characteristic
     */
    void notifyCharacteristicReadReceived(BluetoothGattCharacteristic characteristic);

    /**
     * callback when characteristic change event happens
     *
     * @param characteristic
     */
    void notifyCharacteristicChangeReceived(BluetoothGattCharacteristic characteristic);

    /**
     * callback called when result of characteristic write is received
     *
     * @param characteristic
     */
    void notifyCharacteristicWriteReceived(BluetoothGattCharacteristic characteristic);
}
