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

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

import java.util.List;

/**
 * Some gatt processing useful functions
 *
 * @author Bertrand Martel
 */
public class GattUtils {

    /**
     * Check if list of service contains a specific characteristic
     *
     * @param serviceList
     * @param characteristicUid
     * @return
     */
    @SuppressLint("NewApi")
    public static boolean hasCharacteristic(List<BluetoothGattService> serviceList, String characteristicUid) {

        for (int i = 0; i < serviceList.size(); i++) {
            for (int j = 0; j < serviceList.get(i).getCharacteristics().size(); j++) {
                if (serviceList.get(i).getCharacteristics().get(j).getUuid().toString().equals(characteristicUid)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Retrieve gatt characteristic object from service list
     *
     * @param serviceList
     * @param characteristicUid
     * @return
     */
    @SuppressLint("NewApi")
    public static BluetoothGattCharacteristic getCharacteristic(List<BluetoothGattService> serviceList, String characteristicUid) {

        for (int i = 0; i < serviceList.size(); i++) {
            for (int j = 0; j < serviceList.get(i).getCharacteristics().size(); j++) {
                if (serviceList.get(i).getCharacteristics().get(j).getUuid().toString().equals(characteristicUid)) {
                    return serviceList.get(i).getCharacteristics().get(j);
                }
            }
        }
        return null;
    }

    /**
     * Retrieve gatt descriptorUid object from service list
     *
     * @param serviceList
     * @param descriptorUid
     * @return
     */
    @SuppressLint("NewApi")
    public static BluetoothGattDescriptor getDescriptorForCharac(List<BluetoothGattService> serviceList, String characteristicUid, String descriptorUid) {

        BluetoothGattCharacteristic charac = getCharacteristic(serviceList, characteristicUid);

        if (charac != null) {

            for (int k = 0; k < charac.getDescriptors().size(); k++) {
                if (charac.getDescriptors().get(k).getUuid().toString().equals(descriptorUid)) {
                    return charac.getDescriptors().get(k);
                }
            }

        } else {
            Log.e("GattUtils", "characteristic with uid " + characteristicUid + " is inconsistent");
        }
        return null;
    }

}
