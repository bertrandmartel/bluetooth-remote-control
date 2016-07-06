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
package com.github.akinaru.bledisplayremote.service;

import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

import com.github.akinaru.bledisplayremote.bluetooth.BluetoothCustomManager;
import com.github.akinaru.bledisplayremote.bluetooth.connection.IBluetoothDeviceConn;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

/**
 * Service persisting bluetooth connection
 *
 * @author Bertrand Martel
 */
public class BleDisplayRemoteService extends Service {

    private String TAG = BleDisplayRemoteService.class.getSimpleName();

    /**
     * Service binder
     */
    private final IBinder mBinder = new LocalBinder();

    /*
     * LocalBInder that render public getService() for public access
     */
    public class LocalBinder extends Binder {
        public BleDisplayRemoteService getService() {
            return BleDisplayRemoteService.this;
        }
    }

    private BluetoothCustomManager btManager = null;

    private ScheduledExecutorService executor;

    @Override
    public void onCreate() {

        //initiate bluetooth manager object used to manage all Android Bluetooth API
        btManager = new BluetoothCustomManager(this);

        //initialize bluetooth adapter
        btManager.init(this);
        executor = Executors.newScheduledThreadPool(1);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    /**
     * retrieve scanning list
     *
     * @return
     */
    public Map<String, BluetoothDevice> getScanningList() {
        return btManager.getScanningList();
    }

    /**
     * get scanning state
     *
     * @return
     */
    public boolean isScanning() {
        return btManager.isScanning();
    }

    /**
     * stop bluetooth scan
     */
    public void stopScan() {
        btManager.stopScan();
    }

    /**
     * connect to a bluetooth device
     *
     * @param deviceAddress
     */
    public void connect(String deviceAddress) {
        btManager.connect(deviceAddress);
    }

    /**
     * start Bluetooth scan
     *
     * @return
     */
    public boolean startScan() {
        return btManager.scanLeDevice();
    }

    /**
     * clear bluetooth scanning list
     */
    public void clearScanningList() {
        btManager.clearScanningList();
    }

    /**
     * disconnect a Bluetooth device by address
     *
     * @param deviceAddress bluetooth device address
     * @return true if disconection is successfull
     */
    public boolean disconnect(String deviceAddress) {
        return btManager.disconnect(deviceAddress);
    }

    /**
     * disconnect all bluetooth devices
     */
    public void disconnectall() {
        btManager.disconnectAll();
    }

    /**
     * Retrieve all devices associated
     *
     * @return
     */
    public HashMap<String, IBluetoothDeviceConn> getConnectionList() {
        return btManager.getConnectionList();
    }

}