/****************************************************************************
 * This file is part of Bluetooth LE Analyzer.                              *
 * <p/>                                                                     *
 * Copyright (C) 2016  Bertrand Martel                                      *
 * <p/>                                                                     *
 * Foobar is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 * <p/>                                                                     *
 * Foobar is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 * <p/>                                                                     *
 * You should have received a copy of the GNU General Public License        *
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.          *
 */
package com.github.akinaru.bledisplayremote.bluetooth;

import android.bluetooth.BluetoothGattCharacteristic;

import com.github.akinaru.bledisplayremote.bluetooth.listener.IDeviceInitListener;

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
