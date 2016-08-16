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
import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import com.github.akinaru.bleremote.bluetooth.connection.BluetoothDeviceConn;
import com.github.akinaru.bleremote.bluetooth.connection.IBluetoothDeviceConn;
import com.github.akinaru.bleremote.bluetooth.events.BluetoothEvents;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bleremote.constant.JsonConstants;
import com.github.akinaru.bleremote.utils.ManualResetEvent;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


/**
 * Bluetooth android API processing : contains all android bluetooth api
 * <p/>
 * alternative to this is using an Android Service that you can bind to your main activity
 *
 * @author Bertrand Martel
 */
public class BluetoothCustomManager implements IBluetoothCustomManager {

    private final static String TAG = BluetoothCustomManager.class.getName();

    // set init pool size
    private static final int CORE_POOL_SIZE = 1;

    // set max pool size
    private static final int MAXIMUM_POOL_SIZE = 1;

    // Sets the amount of time an idle thread will wait for a task before terminating
    private static final int KEEP_ALIVE_TIME = 5;

    // set time unit in seconds
    private static final TimeUnit KEEP_ALIVE_TIME_UNIT = TimeUnit.SECONDS;

    LinkedBlockingQueue gattWorkingQueue = new LinkedBlockingQueue<Runnable>();

    /*
     * Creates a new pool of Thread objects for the download work queue
     */
    ThreadPoolExecutor gattThreadPool = new ThreadPoolExecutor(CORE_POOL_SIZE, MAXIMUM_POOL_SIZE,
            KEEP_ALIVE_TIME, KEEP_ALIVE_TIME_UNIT, gattWorkingQueue);

    /**
     * timeout for waiting for response frame from the device
     */
    private final static int BT_TIMEOUT = 200;

    /**
     * set bluetooth scan period
     */
    private final int SCAN_PERIOD = 30000;

    /**
     * list of bluetooth connection by address
     */
    private HashMap<String, IBluetoothDeviceConn> bluetoothConnectionList = new HashMap<>();

    private HashMap<String, BluetoothDevice> scanningList = new HashMap<>();

    /**
     * event manager used to block / release process
     */
    private ManualResetEvent eventManager = new ManualResetEvent(false);

    /**
     * Bluetooth adapter
     */
    private BluetoothAdapter mBluetoothAdapter = null;

    /**
     * set bluetooth scan
     */
    private volatile boolean scanning = false;

    /**
     * Callback for Bluetooth adapter
     * This will be called when a bluetooth device has been discovered
     */
    private BluetoothAdapter.LeScanCallback scanCallback = null;

    private Context context = null;

    private ScheduledExecutorService executor = Executors.newScheduledThreadPool(1);

    private HashMap<String, ScheduledFuture<?>> waitingForDisconnectionList = new HashMap<>();

    /**
     * Build bluetooth manager
     */
    public BluetoothCustomManager(Context context) {
        this.context = context;
    }


    @SuppressLint("NewApi")
    public void init(Context context) {

        // Initializes Bluetooth adapter.
        final BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);

        mBluetoothAdapter = bluetoothManager.getAdapter();

        scanCallback = new BluetoothAdapter.LeScanCallback() {

            @Override
            public void onLeScan(BluetoothDevice device, int rssi, final byte[] scanRecord) {

                if (device.getAddress() != null &&
                        device.getName() != null) {
                    dispatchBtDevices(device, rssi, scanRecord);
                }
            }
        };
    }

    private void dispatchBtDevices(BluetoothDevice device, int rssi, final byte[] scanRecord) {

        if (!scanningList.containsKey(device.getAddress())) {

            Log.v(TAG, "found a new Bluetooth device : " + device.getName() + " : " + device.getAddress());

            scanningList.put(device.getAddress(), device);

            try {
                JSONObject object = new JSONObject();
                object.put(JsonConstants.BT_ADDRESS, device.getAddress());
                object.put(JsonConstants.BT_DEVICE_NAME, device.getName());
                object.put(JsonConstants.BT_ADVERTISING_INTERVAL, -1);

                ArrayList<String> deviceInfo = new ArrayList<>();
                deviceInfo.add(object.toString());

                broadcastUpdateStringList(BluetoothEvents.BT_EVENT_DEVICE_DISCOVERED, deviceInfo);

            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * clear list adapter (usually before rescanning)
     */
    public void clearScanningList() {
        scanningList.clear();
    }

    /**
     * Scan new Bluetooth device
     */
    @SuppressLint("NewApi")
    public boolean scanLeDevice() {

        if (!scanning) {

            broadcastUpdate(BluetoothEvents.BT_EVENT_SCAN_START);

            scanning = true;

            return mBluetoothAdapter.startLeScan(scanCallback);
        }
        return false;
    }

    /**
     * Stop Bluetooth LE scanning
     */
    @SuppressLint("NewApi")
    public void stopScan() {
        scanning = false;
        mBluetoothAdapter.stopLeScan(scanCallback);
        //notify end of scan
        broadcastUpdate(BluetoothEvents.BT_EVENT_SCAN_END);
    }

    public boolean isScanning() {
        return scanning;
    }

    /**
     * Connect to device's GATT server
     */
    @SuppressLint("NewApi")
    public boolean connect(String address) {
        if (mBluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);

        boolean alreadyInList = false;

        if (bluetoothConnectionList.containsKey(address)) {
            alreadyInList = true;
        }

        if (alreadyInList) {

            Log.v(TAG, "reusing same connection");

            BluetoothDeviceConn conn = (BluetoothDeviceConn) bluetoothConnectionList.get(address);

            conn.setGatt(device.connectGatt(context, false, conn.getGattCallback()));

        } else {

            BluetoothDeviceConn conn = new BluetoothDeviceConn(address, device.getName(), this);

            bluetoothConnectionList.put(address, conn);

            Log.v(TAG, "new connection");
            //connect to gatt server on the device
            conn.setGatt(device.connectGatt(context, false, conn.getGattCallback()));
        }

        return true;
    }

    @Override
    public ManualResetEvent getEventManager() {
        return eventManager;
    }

    /**
     * Send broadcast data through broadcast receiver
     *
     * @param action action to be sent
     */
    @Override
    public void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        context.sendBroadcast(intent);
    }

    /**
     * broadcast characteristic value
     *
     * @param action action to be sent (data available)
     */
    @Override
    public void broadcastUpdateStringList(String action, ArrayList<String> valueList) {

        String valueName = "";
        final Intent intent = new Intent(action);
        intent.putStringArrayListExtra(valueName, valueList);
        context.sendBroadcast(intent);
    }

    @SuppressLint("NewApi")
    @Override
    public void writeCharacteristic(String characUid, byte[] value, BluetoothGatt gatt, IPushListener listener, final boolean noResponse) {

        if (gatt != null && characUid != null && value != null) {

            gattThreadPool.execute(new GattTask(gatt, characUid, value, listener) {
                @Override
                public void run() {

                    BluetoothGattCharacteristic charac = GattUtils.getCharacteristic(getGatt().getServices(), getUid());
                    if (noResponse)
                        charac.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    charac.setValue(getValue());

                    getGatt().writeCharacteristic(charac);

                    if (!noResponse) {
                        long startTime = System.currentTimeMillis();
                        eventManager.reset();
                        try {
                            eventManager.waitOne(BT_TIMEOUT);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        long endTime = System.currentTimeMillis();

                        if ((endTime - startTime) >= BT_TIMEOUT) {
                            if (getListener() != null) {
                                getListener().onPushFailure();
                            }
                        } else {
                            if (getListener() != null) {
                                getListener().onPushSuccess();
                            }
                        }
                    }
                    else{
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
            });
        } else
            Log.e(TAG, "Error int writeCharacteristic() input argument NULL");
    }

    @SuppressLint("NewApi")
    @Override
    public void readCharacteristic(String characUid, BluetoothGatt gatt) {

        if (gatt != null && characUid != null) {

            gattThreadPool.execute(new GattTask(gatt, characUid, null, null) {
                @Override
                public void run() {

                    BluetoothGattCharacteristic charac = GattUtils.getCharacteristic(getGatt().getServices(), getUid());

                    getGatt().readCharacteristic(charac);
                    eventManager.reset();
                    try {
                        eventManager.waitOne(BT_TIMEOUT);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
        } else
            Log.e(TAG, "Error int writeCharacteristic() input argument NULL");
    }

    @SuppressLint("NewApi")
    @Override
    public void writeDescriptor(String descriptorUid, BluetoothGatt gatt, byte[] value, String serviceUid, String characUid) {

        if (gatt != null && descriptorUid != null) {

            gattThreadPool.execute(new GattTask(gatt, descriptorUid, value, serviceUid, characUid) {
                @Override
                public void run() {

                    if (getGatt().getService(UUID.fromString(getDescriptorServiceUid())) != null &&
                            getGatt().getService(UUID.fromString(getDescriptorServiceUid())).getCharacteristic(UUID.fromString(getDescriptorCharacUid())) != null &&
                            getGatt().getService(UUID.fromString(getDescriptorServiceUid())).getCharacteristic(UUID.fromString(getDescriptorCharacUid())).getDescriptor(UUID.fromString(getUid())) != null) {

                        BluetoothGattDescriptor descriptor = getGatt().getService(UUID.fromString(getDescriptorServiceUid()))
                                .getCharacteristic(UUID.fromString(getDescriptorCharacUid())).getDescriptor(UUID.fromString(getUid()));

                        descriptor.setValue(getValue());

                        getGatt().writeDescriptor(descriptor);
                    }
                    eventManager.reset();
                    try {
                        eventManager.waitOne(BT_TIMEOUT);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
            gattThreadPool.execute(new Runnable() {
                @Override
                public void run() {

                }
            });
        } else
            Log.e(TAG, "Error int writeCharacteristic() input argument NULL");
    }

    @Override
    public HashMap<String, IBluetoothDeviceConn> getConnectionList() {
        return bluetoothConnectionList;
    }

    @Override
    public HashMap<String, ScheduledFuture<?>> getWaitingMap() {
        return waitingForDisconnectionList;
    }

    @Override
    public void writeLongCharacteristic(String charac, byte[] data, BluetoothGatt gatt, IPushListener listener) {
        if (gatt != null && charac != null && data != null) {

            gattThreadPool.execute(new GattTask(gatt, charac, data, listener) {
                @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void run() {

                    getGatt().beginReliableWrite();

                    BluetoothGattCharacteristic charac = GattUtils.getCharacteristic(getGatt().getServices(), getUid());
                    //charac.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    charac.setValue(getValue());

                    getGatt().writeCharacteristic(charac);

                    long startTime = System.currentTimeMillis();
                    eventManager.reset();
                    try {
                        eventManager.waitOne(BT_TIMEOUT);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    long endTime = System.currentTimeMillis();

                    getGatt().executeReliableWrite();
                    if ((endTime - startTime) >= BT_TIMEOUT) {
                        if (getListener() != null) {
                            getListener().onPushFailure();
                        }
                    } else {
                        if (getListener() != null) {
                            getListener().onPushSuccess();
                        }
                    }
                }
            });
        } else
            Log.e(TAG, "Error int writeCharacteristic() input argument NULL");
    }

    @SuppressLint("NewApi")
    public boolean disconnect(final String deviceAddress) {

        if (mBluetoothAdapter == null || deviceAddress == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        if (bluetoothConnectionList.containsKey(deviceAddress)) {

            if (bluetoothConnectionList.get(deviceAddress).getBluetoothGatt() != null) {
                Log.v(TAG, "disconnect device");
                bluetoothConnectionList.get(deviceAddress).getBluetoothGatt().disconnect();

                if (!waitingForDisconnectionList.containsKey(deviceAddress)) {

                    ScheduledFuture<?> task = executor.schedule(new Runnable() {
                        @Override
                        public void run() {
                            Log.v(TAG, "connection forced close");
                            bluetoothConnectionList.get(deviceAddress).getBluetoothGatt().close();
                            waitingForDisconnectionList.remove(deviceAddress);
                        }
                    }, 1000, TimeUnit.MILLISECONDS);

                    waitingForDisconnectionList.put(deviceAddress, task);
                }
                bluetoothConnectionList.get(deviceAddress).setConnected(false);
            }

            return true;
        } else {
            Log.e(TAG, "device " + deviceAddress + " not found in list");
        }
        return false;
    }

    public void disconnectAll() {
        Iterator it = getConnectionList().entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<String, IBluetoothDeviceConn> pair = (Map.Entry) it.next();
            pair.getValue().disconnect();
        }
    }

    public HashMap<String, BluetoothDevice> getScanningList() {
        return scanningList;
    }
}
