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
package com.github.akinaru.bleremote.bluetooth.events;

import android.content.Intent;

import com.github.akinaru.bleremote.constant.JsonConstants;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

/**
 * Object used to wrap broadcast intent json output
 *
 * @author Bertrand Martel
 */
public class BluetoothObject {

    private String deviceAddress = "";

    private String deviceName = "";

    public BluetoothObject(String deviceAddress, String deviceName, int advertizingInterval) {
        this.deviceAddress = deviceAddress;
        this.deviceName = deviceName;
    }

    public static BluetoothObject parseArrayList(Intent intent) {

        ArrayList<String> actionsStr = intent.getStringArrayListExtra("");

        if (actionsStr.size() > 0) {

            try {

                JSONObject mainObject = new JSONObject(actionsStr.get(0));

                if (mainObject.has(JsonConstants.BT_ADDRESS) &&
                        mainObject.has(JsonConstants.BT_DEVICE_NAME)) {

                    int scanInterval = -1;
                    if (mainObject.has(JsonConstants.BT_ADVERTISING_INTERVAL))
                        scanInterval = mainObject.getInt(JsonConstants.BT_ADVERTISING_INTERVAL);

                    return new BluetoothObject(mainObject.get(JsonConstants.BT_ADDRESS).toString(),
                            mainObject.get(JsonConstants.BT_DEVICE_NAME).toString(),
                            scanInterval);
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    public String getDeviceAddress() {
        return deviceAddress;
    }

    public String getDeviceName() {
        return deviceName;
    }
}
