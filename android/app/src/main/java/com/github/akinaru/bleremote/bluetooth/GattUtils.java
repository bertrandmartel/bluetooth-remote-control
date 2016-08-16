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
