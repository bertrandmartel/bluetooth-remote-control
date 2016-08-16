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
