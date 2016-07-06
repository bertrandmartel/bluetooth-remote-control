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

import android.bluetooth.BluetoothGatt;

import com.github.akinaru.bledisplayremote.bluetooth.listener.IPushListener;

/**
 * Description of a GATT task to be executed (write/read characteristic/descriptor)
 *
 * @author Bertrand Martel
 */
public abstract class GattTask implements Runnable {

    private String gattUid = "";
    private byte[] value = null;

    private String descriptorCharacUid = "";
    private String descriptorServiceUid = "";
    private IPushListener listener = null;

    private BluetoothGatt gatt = null;

    public GattTask(BluetoothGatt gatt, String descriptorUid, byte[] descriptorVal, String serviceUid, String characUid) {
        this.gatt = gatt;
        this.gattUid = descriptorUid;
        this.value = descriptorVal;
        this.descriptorCharacUid = characUid;
        this.descriptorServiceUid = serviceUid;
    }

    public GattTask(BluetoothGatt gatt, String gattUid, byte[] value, IPushListener listener) {
        this.gatt = gatt;
        this.gattUid = gattUid;
        this.value = value;
        this.listener = listener;
    }

    public String getUid() {
        return gattUid;
    }

    public byte[] getValue() {
        return value;
    }

    public String getDescriptorServiceUid() {
        return descriptorServiceUid;
    }

    public String getDescriptorCharacUid() {
        return descriptorCharacUid;
    }

    public BluetoothGatt getGatt() {
        return gatt;
    }

    public IPushListener getListener() {
        return listener;
    }

}
