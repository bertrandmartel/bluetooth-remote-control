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

import android.bluetooth.BluetoothGatt;

import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;

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
