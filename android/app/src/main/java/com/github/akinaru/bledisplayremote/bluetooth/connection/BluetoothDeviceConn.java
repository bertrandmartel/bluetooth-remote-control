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
package com.github.akinaru.bledisplayremote.bluetooth.connection;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothProfile;
import android.util.Log;

import com.github.akinaru.bledisplayremote.bluetooth.IBluetoothCustomManager;
import com.github.akinaru.bledisplayremote.bluetooth.IDevice;
import com.github.akinaru.bledisplayremote.bluetooth.device.BleDisplayDevice;
import com.github.akinaru.bledisplayremote.bluetooth.events.BluetoothEvents;
import com.github.akinaru.bledisplayremote.bluetooth.listener.IDeviceInitListener;
import com.github.akinaru.bledisplayremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bledisplayremote.constant.JsonConstants;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.UUID;
import java.util.concurrent.ExecutorService;

/**
 * Bluetooth device connection management
 *
 * @author Bertrand Martel
 */
public class BluetoothDeviceConn implements IBluetoothDeviceConn {

    private final static String TAG = BluetoothDeviceConn.class.getName();

    public final static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";

    /**
     * Bluetooth callback for gatt layer interaction
     */
    private BluetoothGattCallback gattCallback = null;

    /**
     * bluetooth gatt connection object
     */
    private BluetoothGatt gatt = null;

    /**
     * device address
     */
    private String deviceAddr = "";

    private String deviceName = "";

    private IBluetoothCustomManager manager = null;

    private IDevice device = null;

    private boolean connected = false;

    /**
     * Build Bluetooth device connection
     *
     * @param address
     */
    @SuppressLint("NewApi")
    public BluetoothDeviceConn(String address, String deviceName, final IBluetoothCustomManager manager) {
        this.deviceAddr = address;
        this.deviceName = deviceName;
        this.manager = manager;

        gattCallback = new BluetoothGattCallback() {
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status,
                                                int newState) {

                if (newState == BluetoothProfile.STATE_CONNECTED) {

                    Log.i(TAG, "Connected to GATT server.");
                    Log.i(TAG, "Attempting to start service discovery:" + gatt.discoverServices());

                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {

                    connected = false;
                    Log.i(TAG, "Disconnected from GATT server.");

                    try {
                        JSONObject object = new JSONObject();
                        object.put(JsonConstants.BT_ADDRESS, getAddress());
                        object.put(JsonConstants.BT_DEVICE_NAME, getDeviceName());

                        ArrayList<String> values = new ArrayList<String>();
                        values.add(object.toString());

                        //when device is fully intitialized broadcast service discovery
                        manager.broadcastUpdateStringList(BluetoothEvents.BT_EVENT_DEVICE_DISCONNECTED, values);
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }

                    if (manager.getWaitingMap().containsKey(deviceAddr)) {
                        manager.getWaitingMap().get(deviceAddr).cancel(true);
                        manager.getWaitingMap().remove(deviceAddr);
                    }

                    if (BluetoothDeviceConn.this.gatt != null) {
                        Log.i(TAG, "connection close clean");
                        BluetoothDeviceConn.this.gatt.close();
                    }
                }
            }

            @Override
            // New services discovered
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {

                if (status == BluetoothGatt.GATT_SUCCESS) {

                    Runnable test = new Runnable() {
                        @Override
                        public void run() {

                            //you can improve this by using reflection
                            device = new BleDisplayDevice(BluetoothDeviceConn.this);

                            device.addInitListener(new IDeviceInitListener() {
                                @Override
                                public void onInit() {
                                    try {
                                        JSONObject object = new JSONObject();
                                        object.put(JsonConstants.BT_ADDRESS, getAddress());
                                        object.put(JsonConstants.BT_DEVICE_NAME, getDeviceName());

                                        ArrayList<String> values = new ArrayList<String>();
                                        values.add(object.toString());

                                        connected = true;
                                        //when device is fully intitialized broadcast service discovery
                                        manager.broadcastUpdateStringList(BluetoothEvents.BT_EVENT_DEVICE_CONNECTED, values);
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                }
                            });
                            device.init();
                        }
                    };
                    Thread testThread = new Thread(test);
                    testThread.start();

                } else {
                    Log.w(TAG, "onServicesDiscovered received: " + status);
                }
            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                manager.getEventManager().set();
                if (device != null) {
                    device.notifyCharacteristicWriteReceived(characteristic);
                }
            }

            @Override
            // Result of a characteristic read operation
            public void onCharacteristicRead(BluetoothGatt gatt,
                                             BluetoothGattCharacteristic characteristic,
                                             int status) {
                manager.getEventManager().set();
                if (device != null) {
                    Log.i(TAG, "onCharacteristicRead");
                    device.notifyCharacteristicReadReceived(characteristic);
                }
            }

            @Override
            public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
                manager.getEventManager().set();
            }

            @Override
            // Characteristic notification
            public void onCharacteristicChanged(BluetoothGatt gatt,
                                                BluetoothGattCharacteristic characteristic) {
                manager.getEventManager().set();
                if (device != null) {
                    Log.i(TAG, "onCharacteristicChanged");
                    device.notifyCharacteristicChangeReceived(characteristic);
                }
            }
        };
    }

    public BluetoothGattCallback getGattCallback() {
        return gattCallback;
    }

    @Override
    public String getAddress() {
        return this.deviceAddr;
    }

    @Override
    public String getDeviceName() {
        return deviceName;
    }

    @Override
    public BluetoothGatt getBluetoothGatt() {
        return gatt;
    }

    @Override
    public boolean isConnected() {
        return connected;
    }

    @SuppressLint("NewApi")
    @Override
    public void writeCharacteristic(String service, String charac, byte[] value, IPushListener listener, boolean noResponse) {
        manager.writeCharacteristic(charac, value, gatt, listener, noResponse);
    }

    @SuppressLint("NewApi")
    @Override
    public void readCharacteristic(String service, String charac) {
        manager.readCharacteristic(charac, gatt);
    }

    @SuppressLint("NewApi")
    @Override
    public void enableDisableNotification(UUID service, UUID charac, boolean enable) {

        if (gatt.getService(service) != null &&
                gatt.getService(service).getCharacteristic(charac) != null)
            gatt.setCharacteristicNotification(gatt.getService(service).getCharacteristic(charac), enable);
        else {
            Log.e(TAG, "error inconsistent service or characteristic");
        }
    }

    @SuppressLint("NewApi")
    @Override
    public void enableGattNotifications(String serviceUid, String characUid) {

        String descriptorStr = CLIENT_CHARACTERISTIC_CONFIG;
        manager.writeDescriptor(descriptorStr, gatt, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE, serviceUid, characUid);
    }

    public BluetoothGatt getGatt() {
        return gatt;
    }

    public void setGatt(BluetoothGatt gatt) {
        this.gatt = gatt;
    }

    @Override
    public IBluetoothCustomManager getManager() {
        return manager;
    }

    @Override
    public IDevice getDevice() {
        return device;
    }

    @SuppressLint("NewApi")
    @Override
    public void disconnect() {
        if (gatt != null) {
            gatt.disconnect();
        }
    }

    @Override
    public void setConnected(boolean state) {
        connected = state;
    }

    @Override
    public void writeLongCharacteristic(String service, String charac, byte[] data, IPushListener listener) {
        manager.writeLongCharacteristic(charac, data, gatt, listener);
    }
}
