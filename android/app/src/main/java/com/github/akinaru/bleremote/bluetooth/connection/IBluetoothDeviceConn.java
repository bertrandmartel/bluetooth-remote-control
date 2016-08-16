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

import android.bluetooth.BluetoothGatt;

import com.github.akinaru.bleremote.bluetooth.IBluetoothCustomManager;
import com.github.akinaru.bleremote.bluetooth.IDevice;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;

import java.util.UUID;

/**
 * Generic template for bluetooth device gatt connection
 *
 * @author Bertrand Martel
 */
public interface IBluetoothDeviceConn {

    /**
     * retrieve bluetooth device address
     *
     * @return
     */
    String getAddress();

    String getDeviceName();

    BluetoothGatt getBluetoothGatt();

    boolean isConnected();

    /**
     * write to a characteristic
     *
     * @param serviceSmartliteControlUUID
     * @param characteristicSmartliteSettingsUUID
     * @param value
     */
    void writeCharacteristic(String serviceSmartliteControlUUID, String characteristicSmartliteSettingsUUID, byte[] value, IPushListener listener,boolean noResponse);

    /**
     * read from a characteristic
     *
     * @param serviceName
     * @param characteristicName
     */
    void readCharacteristic(String serviceName, String characteristicName);

    void enableDisableNotification(UUID service, UUID charac, boolean enable);

    void enableGattNotifications(String service, String charac);

    IBluetoothCustomManager getManager();

    IDevice getDevice();

    void disconnect();

    void setConnected(boolean state);

    void writeLongCharacteristic(String service, String charac, byte[] data, IPushListener listener);
}
