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
package com.github.akinaru.bleremote.bluetooth.device;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothGattCharacteristic;
import android.util.Log;

import com.github.akinaru.bleremote.bluetooth.connection.BluetoothDeviceAbstr;
import com.github.akinaru.bleremote.bluetooth.connection.IBluetoothDeviceConn;
import com.github.akinaru.bleremote.bluetooth.listener.ICharacteristicListener;
import com.github.akinaru.bleremote.bluetooth.listener.IDeviceInitListener;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bleremote.inter.IBleDisplayRemoteDevice;
import com.github.akinaru.bleremote.inter.IButtonListener;
import com.github.akinaru.bleremote.inter.IDirectionPadListener;
import com.github.akinaru.bleremote.model.Button;
import com.github.akinaru.bleremote.model.ButtonState;
import com.github.akinaru.bleremote.model.DpadState;
import com.github.akinaru.bleremote.model.TransmitState;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * RFduino Bluetooth device management
 *
 * @author Bertrand Martel
 */
public class BleDisplayDevice extends BluetoothDeviceAbstr implements IBleDisplayRemoteDevice {

    private String TAG = BleDisplayDevice.this.getClass().getName();

    private final static String SERVICE_BUTTON = "00001523-1212-efde-1523-785feabcd123";
    private final static String BUTTON1 = "00001601-1212-efde-1523-785feabcd123";
    private final static String BUTTON2 = "00001602-1212-efde-1523-785feabcd123";
    private final static String BUTTON3 = "00001603-1212-efde-1523-785feabcd123";
    private final static String BUTTON4 = "00001604-1212-efde-1523-785feabcd123";
    private final static String DPAD = "00001605-1212-efde-1523-785feabcd123";

    private final static String LED = "00001701-1212-efde-1523-785feabcd123";
    private final static String FULL_COLOR = "00001801-1212-efde-1523-785feabcd123";
    private final static String BITMAP = "00001802-1212-efde-1523-785feabcd123";
    private final static String TRANSMIT_STATUS = "00001803-1212-efde-1523-785feabcd123";

    private ArrayList<IDeviceInitListener> initListenerList = new ArrayList<>();

    private ArrayList<IButtonListener> buttonListeners = new ArrayList<>();

    private ArrayList<IDirectionPadListener> dpadListeners = new ArrayList<>();

    private final static int SENDING_BUFFER_MAX_LENGTH = 18;

    private boolean init = false;

    private int sendIndex = 0;
    private int sendingNum = 0;
    private boolean remain = false;
    private byte[] bitmapData;

    private boolean stopProcessingBitmap = true;
    private long dateProcessBegin = 0;
    private int failCount = 0;
    private int frameNumToSend = 0;

    /*
         * Creates a new pool of Thread objects for the download work queue
         */
    ExecutorService threadPool;

    /**
     * @param conn
     */
    @SuppressLint("NewApi")
    public BleDisplayDevice(IBluetoothDeviceConn conn) {
        super(conn);

        threadPool = Executors.newFixedThreadPool(1);

        setCharacteristicListener(new ICharacteristicListener() {

            @Override
            public void onCharacteristicReadReceived(BluetoothGattCharacteristic charac) {

            }

            @Override
            public void onCharacteristicChangeReceived(BluetoothGattCharacteristic charac) {

                Log.i(TAG, "receive something : " + charac.getUuid().toString() + " " + charac.getValue().length + " " + charac.getValue()[0]);

                if (charac.getUuid().toString().equals(TRANSMIT_STATUS)) {

                    if (charac.getValue().length > 0) {

                        TransmitState state = TransmitState.getTransmitState(charac.getValue()[0]);

                        switch (state) {

                            case TRANSMIT_OK:
                                if (sendIndex != sendingNum) {
                                    Log.i(TAG, "received TRANSMIT_OK sending next batch of 128 frames");

                                    threadPool.execute(new Runnable() {
                                        @Override
                                        public void run() {
                                            BleDisplayDevice.this.conn.writeCharacteristic(SERVICE_BUTTON, TRANSMIT_STATUS, new byte[]{(byte) TransmitState.TRANSMITTING.ordinal()}, new IPushListener() {
                                                @Override
                                                public void onPushFailure() {
                                                    Log.e(TAG, "error happenend setting bitmap length");
                                                }

                                                @Override
                                                public void onPushSuccess() {
                                                    Log.i(TAG, "set bitmap length successfull");
                                                    frameNumToSend = 128;
                                                    sendBitmapSequence();
                                                }
                                            }, false);
                                        }
                                    });
                                } else {
                                    Log.i(TAG, "sending is over. Waiting for complete");
                                }
                                break;
                            case TRANSMIT_COMPLETE:
                                Log.i(TAG, "received TRANSMIT_COMPLETE");
                                clearBimapInfo();
                                break;
                        }
                    }
                } else {
                    Button button = Button.NONE;
                    DpadState dpad = DpadState.NONE;

                    switch (charac.getUuid().toString()) {
                        case BUTTON1:
                            button = Button.BUTTON1;
                            break;
                        case BUTTON2:
                            button = Button.BUTTON2;
                            break;
                        case BUTTON3:
                            button = Button.BUTTON3;
                            break;
                        case BUTTON4:
                            button = Button.BUTTON4;
                            break;
                        case DPAD:
                            dpad = DpadState.getDpad(charac.getValue()[0]);
                            break;
                    }
                    if (button != Button.NONE) {
                        ButtonState state = ButtonState.RELEASED;

                        if (charac.getValue().length > 0 && charac.getValue()[0] == 0x01) {
                            state = ButtonState.PRESSED;
                        }
                        for (int i = 0; i < buttonListeners.size(); i++) {
                            if (buttonListeners.get(i) != null) {
                                buttonListeners.get(i).onButtonStateChange(button, state);
                            }
                        }
                    } else {
                        for (int i = 0; i < dpadListeners.size(); i++) {
                            if (dpadListeners.get(i) != null) {
                                dpadListeners.get(i).onDPadStateChanged(dpad);
                            }
                        }
                    }
                }
            }

            @Override
            public void onCharacteristicWriteReceived(BluetoothGattCharacteristic charac) {

            }
        });
    }

    @Override
    public void init() {

        Log.i(TAG, "initializing Ble Display Service");

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(BUTTON1), true);
        conn.enableGattNotifications(SERVICE_BUTTON, BUTTON1);

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(BUTTON2), true);
        conn.enableGattNotifications(SERVICE_BUTTON, BUTTON2);

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(BUTTON3), true);
        conn.enableGattNotifications(SERVICE_BUTTON, BUTTON3);

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(BUTTON4), true);
        conn.enableGattNotifications(SERVICE_BUTTON, BUTTON4);

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(DPAD), true);
        conn.enableGattNotifications(SERVICE_BUTTON, DPAD);

        conn.enableDisableNotification(UUID.fromString(SERVICE_BUTTON), UUID.fromString(TRANSMIT_STATUS), true);
        conn.enableGattNotifications(SERVICE_BUTTON, TRANSMIT_STATUS);

        for (int i = 0; i < initListenerList.size(); i++) {
            initListenerList.get(i).onInit();
        }
    }

    @Override
    public boolean isInit() {
        return init;
    }

    @Override
    public void addInitListener(IDeviceInitListener listener) {
        initListenerList.add(listener);
    }

    @Override
    public void addButtonListener(IButtonListener listener) {
        buttonListeners.add(listener);
    }

    @Override
    public void addDirectionPadListener(IDirectionPadListener listener) {
        dpadListeners.add(listener);
    }

    @Override
    public void pushLedState(byte mask, IPushListener listener) {
        conn.writeCharacteristic(SERVICE_BUTTON, LED, new byte[]{mask}, listener, false);
    }

    @Override
    public void pushFullColor(byte red, byte green, byte blue, IPushListener listener) {
        conn.writeCharacteristic(SERVICE_BUTTON, FULL_COLOR, new byte[]{red, green, blue}, listener, false);
    }

    private void clearBimapInfo() {
        sendingNum = 0;
        remain = false;
        sendIndex = 0;
        bitmapData = new byte[]{};
        stopProcessingBitmap = true;
    }

    private void sendBitmapSequence() {

        if (!stopProcessingBitmap) {

            if (sendIndex != sendingNum) {

                //Log.i(TAG, "index : " + sendIndex + " from " + (sendIndex * SENDING_BUFFER_MAX_LENGTH) + " to " + (sendIndex * SENDING_BUFFER_MAX_LENGTH + SENDING_BUFFER_MAX_LENGTH) + " with length of " + bitmapData.length);
                byte[] data = Arrays.copyOfRange(bitmapData, sendIndex * SENDING_BUFFER_MAX_LENGTH, sendIndex * SENDING_BUFFER_MAX_LENGTH + SENDING_BUFFER_MAX_LENGTH);
                sendIndex++;
                final long dateBegin = new Date().getTime();

                conn.writeCharacteristic(SERVICE_BUTTON, BITMAP, data, new IPushListener() {
                    @Override
                    public void onPushFailure() {

                        Log.e(TAG, "error happenend during transmission. Retrying");
 /*
                        sendIndex--;
                        failCount++;
                        sendBitmapSequence();
                        */
                    }

                    @Override
                    public void onPushSuccess() {
                        /*
                        long dateEnd = new Date().getTime();
                        float timeSpan = (dateEnd - dateBegin) / 1000f;
                        float speed = (SENDING_BUFFER_MAX_LENGTH * 8) / timeSpan;
                        Log.i(TAG, "current speed : " + speed + "bps");
                        */
                    }
                }, true);
                Log.i(TAG, "sending... : " + sendIndex + "/" + sendingNum + " : " +
                        (data[0] & 0xFF) + ";" +
                        (data[1] & 0xFF) + ";" +
                        (data[2] & 0xFF) + ";" +
                        (data[3] & 0xFF) + ";" +
                        (data[4] & 0xFF) + ";" +
                        (data[5] & 0xFF) + ";" +
                        (data[6] & 0xFF) + ";" +
                        (data[7] & 0xFF) + ";" +
                        (data[8] & 0xFF) + ";" +
                        (data[9] & 0xFF) + ";" +
                        (data[10] & 0xFF) + ";" +
                        (data[11] & 0xFF) + ";" +
                        (data[12] & 0xFF) + ";" +
                        (data[13] & 0xFF) + ";" +
                        (data[14] & 0xFF) + ";" +
                        (data[15] & 0xFF) + ";" +
                        (data[16] & 0xFF) + ";" +
                        (data[17] & 0xFF));
                frameNumToSend--;

                if (frameNumToSend != 0) {
                    sendBitmapSequence();
                }
            } else {

                if (remain) {

                    int remainNum = bitmapData.length % SENDING_BUFFER_MAX_LENGTH;
                    //Log.i(TAG, "index : " + sendingNum + " from " + (sendingNum * SENDING_BUFFER_MAX_LENGTH) + " to " + (sendingNum * SENDING_BUFFER_MAX_LENGTH + remainNum) + " with length of " + bitmapData.length);
                    byte[] data = Arrays.copyOfRange(bitmapData, sendingNum * SENDING_BUFFER_MAX_LENGTH, sendingNum * SENDING_BUFFER_MAX_LENGTH + remainNum);

                    Log.i(TAG,"sending : " + data.length);

                    conn.writeCharacteristic(SERVICE_BUTTON, BITMAP, data, new IPushListener() {
                        @Override
                        public void onPushFailure() {
                            /*
                            Log.e(TAG, "error happenend during transmission. Retrying");
                            failCount++;
                            sendBitmapSequence();
                            */
                        }

                        @Override
                        public void onPushSuccess() {
                            /*
                            Log.i(TAG, "completly finished in " + (new Date().getTime() - dateProcessBegin) + "ms - fail : " + failCount + " packet count : " + sendingNum);
                            clearBimapInfo();
                            */
                        }
                    }, true);

                    Log.i(TAG,"frameNumToSend : " + frameNumToSend);
                    Log.i(TAG, "completly finished in " + (new Date().getTime() - dateProcessBegin) + "ms - fail : " + failCount + " packet count : " + sendingNum);
                }
            }
        } else {
            Log.i(TAG, "stop processing bitmap");
        }
    }

    private void sendBitmap(final byte[] bitmapData) {
        Log.i(TAG, "send bitmap with length : " + bitmapData.length);

        sendingNum = bitmapData.length / SENDING_BUFFER_MAX_LENGTH;
        remain = false;
        if ((bitmapData.length % SENDING_BUFFER_MAX_LENGTH) != 0) {
            remain = true;
        }
        sendIndex = 0;
        BleDisplayDevice.this.bitmapData = bitmapData;
        stopProcessingBitmap = false;
        dateProcessBegin = new Date().getTime();
        failCount = 0;

        conn.writeCharacteristic(SERVICE_BUTTON, TRANSMIT_STATUS, new byte[]{(byte) TransmitState.TRANSMITTING.ordinal()}, new IPushListener() {
            @Override
            public void onPushFailure() {
                Log.e(TAG, "error happenend setting transmit status");
            }

            @Override
            public void onPushSuccess() {

                conn.writeCharacteristic(SERVICE_BUTTON, BITMAP, new byte[]{(byte) (bitmapData.length >> 8), (byte) bitmapData.length}, new IPushListener() {
                    @Override
                    public void onPushFailure() {
                        Log.e(TAG, "error happenend setting bitmap length");
                    }

                    @Override
                    public void onPushSuccess() {
                        Log.i(TAG, "set bitmap length successfull");

                    }
                }, true);
                frameNumToSend = 127;
                sendBitmapSequence();
            }
        }, false);
    }

    @Override
    public void sendBitmapNoEncoding(final byte[] bitmapData) {
        sendBitmap(bitmapData);
    }

    @Override
    public void sendBitmapEncodedBitmask(final byte[] bitmapData) {
        sendBitmap(bitmapData);
    }
}