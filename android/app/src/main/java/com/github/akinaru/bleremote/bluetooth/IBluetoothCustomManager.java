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

import com.github.akinaru.bleremote.bluetooth.connection.IBluetoothDeviceConn;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bleremote.utils.ManualResetEvent;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.ScheduledFuture;

/**
 * Generic interface for bluetooth custom manager
 *
 * @author Bertrand Martel
 */
public interface IBluetoothCustomManager {

    ManualResetEvent getEventManager();

    void broadcastUpdate(String action);

    void broadcastUpdateStringList(String action, ArrayList<String> strList);

    void writeCharacteristic(String characUid, byte[] value, BluetoothGatt gatt, IPushListener listener,boolean noResponse);

    void readCharacteristic(String characUid, BluetoothGatt gatt);

    void writeDescriptor(String descriptorUid, BluetoothGatt gatt, byte[] value, String serviceUid, String characUid);

    HashMap<String, IBluetoothDeviceConn> getConnectionList();

    HashMap<String, ScheduledFuture<?>> getWaitingMap();

    void writeLongCharacteristic(String charac, byte[] data, BluetoothGatt gatt, IPushListener listener);
}
