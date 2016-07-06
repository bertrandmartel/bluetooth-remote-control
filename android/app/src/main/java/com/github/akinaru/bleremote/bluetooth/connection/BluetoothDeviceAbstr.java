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
