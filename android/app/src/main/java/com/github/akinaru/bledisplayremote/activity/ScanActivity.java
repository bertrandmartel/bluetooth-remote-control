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
package com.github.akinaru.bledisplayremote.activity;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.github.akinaru.bledisplayremote.R;
import com.github.akinaru.bledisplayremote.adapter.ScanItemArrayAdapter;
import com.github.akinaru.bledisplayremote.bluetooth.events.BluetoothEvents;
import com.github.akinaru.bledisplayremote.bluetooth.events.BluetoothObject;
import com.github.akinaru.bledisplayremote.common.SimpleDividerItemDecoration;
import com.github.akinaru.bledisplayremote.inter.IViewHolderClickListener;
import com.github.akinaru.bledisplayremote.service.BleDisplayRemoteService;

import java.util.ArrayList;
import java.util.List;

/**
 * Bluetooth devices scanned activity
 *
 * @author Bertrand Martel
 */
public class ScanActivity extends BaseActivity {

    /**
     * debug tag
     */
    private String TAG = this.getClass().getName();

    /**
     * list of all BLE devices found on network
     */
    private RecyclerView scanningRecyclerView = null;

    /**
     * adapter for scanning recyclerview
     */
    private ScanItemArrayAdapter scanningAdapter = null;

    /**
     * list of filtered HCI packet (this is cleared when filter is cancelled)
     */
    private List<BluetoothObject> scanList = new ArrayList<>();

    /**
     * swipe refresh layout used to make refresh animation when scrolling top of recyclerview
     */
    private SwipeRefreshLayout mSwipeRefreshLayout;

    /**
     * frame layout containing recyclerview
     */
    private FrameLayout mDisplayFrame;

    /**
     * frame layout when nothing to show in recyclerview
     */
    private FrameLayout mWaitingFrame;

    /**
     * define if this is the first device discovered or not
     */
    private boolean mFirstPacketReceived = true;

    protected void onCreate(Bundle savedInstanceState) {

        setLayout(R.layout.activity_scan);
        super.onCreate(savedInstanceState);

        //register bluetooth event broadcast receiver
        registerReceiver(mBluetoothReceiver, makeGattUpdateIntentFilter());

        if (mBluetoothAdapter.isEnabled()) {
            Intent intent = new Intent(this, BleDisplayRemoteService.class);
            mBound = bindService(intent, mServiceConnection, BIND_AUTO_CREATE);
        }

        //setup recyclerview
        scanningRecyclerView = (RecyclerView) findViewById(R.id.scan_list);

        scanList = new ArrayList<>();

        scanningAdapter = new ScanItemArrayAdapter(ScanActivity.this, scanList, new IViewHolderClickListener() {
            @Override
            public void onClick(View v) {

                int index = scanningRecyclerView.getChildAdapterPosition(v);

                BluetoothObject btDevice = scanList.get(index);

                mService.stopScan();

                //launch packet description activity
                Intent intent = new Intent(ScanActivity.this, DeviceActivity.class);
                intent.putExtra("deviceAddress", btDevice.getDeviceAddress());
                intent.putExtra("deviceName", btDevice.getDeviceName());

                startActivity(intent);

            }
        });

        //set layout manager
        scanningRecyclerView.setLayoutManager(new GridLayoutManager(this, 1, LinearLayoutManager.VERTICAL, false));

        //set line decoration
        scanningRecyclerView.addItemDecoration(new SimpleDividerItemDecoration(
                getApplicationContext()
        ));

        scanningRecyclerView.setAdapter(scanningAdapter);

        mDisplayFrame = (FrameLayout) findViewById(R.id.display_frame);
        mWaitingFrame = (FrameLayout) findViewById(R.id.waiting_frame);

        //setup swipe refresh
        mSwipeRefreshLayout = (SwipeRefreshLayout) findViewById(R.id.swiperefresh);
        mSwipeRefreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mService != null) {
                            mService.clearScanningList();
                            mService.startScan();
                        }
                        mFirstPacketReceived = true;
                        mDisplayFrame.setVisibility(View.GONE);
                        mWaitingFrame.setVisibility(View.VISIBLE);
                        scanList.clear();
                        scanningAdapter.notifyDataSetChanged();
                        mSwipeRefreshLayout.setRefreshing(false);
                    }
                });
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == REQUEST_ENABLE_BT) {

            if (mBluetoothAdapter.isEnabled()) {
                Intent intent = new Intent(this, BleDisplayRemoteService.class);
                // bind the service to current activity and create it if it didnt exist before
                startService(intent);
                mBound = bindService(intent, mServiceConnection, BIND_AUTO_CREATE);

            } else {
                Toast.makeText(this, getResources().getString(R.string.toast_bluetooth_disabled), Toast.LENGTH_SHORT).show();
            }
        }
    }


    /**
     * trigger a BLE scan
     */
    public void triggerNewScan() {

        if (mService != null && !mService.isScanning()) {
            Log.v(TAG, "start scan");
            mService.disconnectall();
            mService.startScan();
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                showProgressBar();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "onDestroy");
        unregisterReceiver(mBluetoothReceiver);
        try {
            if (mBound) {
                unbindService(mServiceConnection);
                mBound = false;
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        triggerNewScan();
    }

    @Override
    protected void onPause() {
        super.onPause();

        if (mService != null) {
            if (mService.isScanning()) {
                hideProgressBar();
                mService.stopScan();
            }
        }
    }

    /**
     * broadcast receiver to receive bluetooth events
     */
    private final BroadcastReceiver mBluetoothReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            final String action = intent.getAction();

            if (BluetoothEvents.BT_EVENT_SCAN_START.equals(action)) {
                Log.v(TAG, "Scan has started");
            } else if (BluetoothEvents.BT_EVENT_SCAN_END.equals(action)) {
                Log.v(TAG, "Scan has ended");
            } else if (BluetoothEvents.BT_EVENT_DEVICE_DISCOVERED.equals(action)) {
                Log.v(TAG, "New device has been discovered");
                final BluetoothObject btDeviceTmp = BluetoothObject.parseArrayList(intent);

                if (btDeviceTmp != null) {

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (mFirstPacketReceived) {
                                mFirstPacketReceived = false;
                                //display recyclerview + swipe refresh view
                                mWaitingFrame.setVisibility(View.GONE);
                                mDisplayFrame.setVisibility(View.VISIBLE);
                            }
                            if (scanningAdapter != null) {
                                for (int i = 0; i < scanList.size(); i++) {
                                    if (btDeviceTmp.getDeviceAddress().equals(scanList.get(i).getDeviceAddress()))
                                        return;
                                }
                                scanList.add(btDeviceTmp);
                                scanningAdapter.notifyDataSetChanged();
                            }
                        }
                    });
                }

            } else if (BluetoothEvents.BT_EVENT_DEVICE_DISCONNECTED.equals(action)) {
                Log.v(TAG, "Device disconnected");

            } else if (BluetoothEvents.BT_EVENT_DEVICE_CONNECTED.equals(action)) {
            }
        }
    };

    /**
     * Manage Bluetooth Service
     */
    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {

            Log.v(TAG, "connected to service");
            mService = ((BleDisplayRemoteService.LocalBinder) service).getService();

            mService.clearScanningList();
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mFirstPacketReceived = true;
                    mDisplayFrame.setVisibility(View.GONE);
                    mWaitingFrame.setVisibility(View.VISIBLE);
                    triggerNewScan();
                }
            });
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
        }
    };

    /**
     * add filter to intent to receive notification from bluetooth service
     *
     * @return intent filter
     */
    private static IntentFilter makeGattUpdateIntentFilter() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothEvents.BT_EVENT_SCAN_START);
        intentFilter.addAction(BluetoothEvents.BT_EVENT_SCAN_END);
        intentFilter.addAction(BluetoothEvents.BT_EVENT_DEVICE_DISCOVERED);
        intentFilter.addAction(BluetoothEvents.BT_EVENT_DEVICE_CONNECTED);
        intentFilter.addAction(BluetoothEvents.BT_EVENT_DEVICE_DISCONNECTED);
        return intentFilter;
    }
}