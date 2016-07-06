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
