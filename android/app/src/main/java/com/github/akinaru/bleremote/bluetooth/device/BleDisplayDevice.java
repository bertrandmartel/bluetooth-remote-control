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
import com.github.akinaru.bleremote.inter.IDirectionPadListener;
import com.github.akinaru.bleremote.inter.IProgressListener;
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
    private final static String DPAD = "00001605-1212-efde-1523-785feabcd123";

    private final static String LED = "00001701-1212-efde-1523-785feabcd123";
    private final static String FULL_COLOR = "00001801-1212-efde-1523-785feabcd123";
    private final static String BITMAP = "00001802-1212-efde-1523-785feabcd123";
    private final static String TRANSMIT_STATUS = "00001803-1212-efde-1523-785feabcd123";

    private ArrayList<IDeviceInitListener> initListenerList = new ArrayList<>();

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

    private IProgressListener progressListener;
    private boolean stopUpload = false;
    private boolean mUploading = false;

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

                if (charac.getUuid().toString().equals(TRANSMIT_STATUS)) {

                    if (charac.getValue().length > 0) {

                        TransmitState state = TransmitState.getTransmitState(charac.getValue()[0]);

                        switch (state) {

                            case TRANSMIT_OK:
                                if (sendIndex != sendingNum) {
                                    Log.v(TAG, "received TRANSMIT_OK sending next batch of 128 frames");

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
                                                    Log.v(TAG, "set bitmap length successfull");
                                                    frameNumToSend = 128;
                                                    sendBitmapSequence();
                                                }
                                            }, false);
                                        }
                                    });
                                } else {
                                    Log.v(TAG, "sending is over. Waiting for complete");
                                }
                                break;
                            case TRANSMIT_COMPLETE:
                                Log.v(TAG, "received TRANSMIT_COMPLETE");
                                if (progressListener != null) {
                                    progressListener.onComplete();
                                }
                                clearBimapInfo();
                                mUploading = false;
                                break;
                        }
                    }
                } else {
                    DpadState dpad = DpadState.NONE;

                    switch (charac.getUuid().toString()) {
                        case DPAD:
                            dpad = DpadState.getDpad(charac.getValue()[0]);
                            break;
                    }

                    for (int i = 0; i < dpadListeners.size(); i++) {
                        if (dpadListeners.get(i) != null) {
                            dpadListeners.get(i).onDPadStateChanged(dpad);
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

        Log.v(TAG, "initializing Ble Display Service");

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

                byte[] data = Arrays.copyOfRange(bitmapData, sendIndex * SENDING_BUFFER_MAX_LENGTH, sendIndex * SENDING_BUFFER_MAX_LENGTH + SENDING_BUFFER_MAX_LENGTH);
                sendIndex++;

                conn.writeCharacteristic(SERVICE_BUTTON, BITMAP, data, new IPushListener() {
                    @Override
                    public void onPushFailure() {

                        Log.e(TAG, "error happenend during transmission. Retrying");
                    }

                    @Override
                    public void onPushSuccess() {

                    }
                }, true);

                if (progressListener != null) {
                    progressListener.onProgress((sendIndex * 100) / sendingNum);
                }

                Log.v(TAG, "sending... : " + sendIndex + "/" + sendingNum + " : " +
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

                if (frameNumToSend != 0 && !stopUpload) {
                    sendBitmapSequence();
                }
            } else {

                if (remain) {

                    int remainNum = bitmapData.length % SENDING_BUFFER_MAX_LENGTH;

                    byte[] data = Arrays.copyOfRange(bitmapData, sendingNum * SENDING_BUFFER_MAX_LENGTH, sendingNum * SENDING_BUFFER_MAX_LENGTH + remainNum);

                    Log.v(TAG, "sending : " + data.length);

                    conn.writeCharacteristic(SERVICE_BUTTON, BITMAP, data, new IPushListener() {
                        @Override
                        public void onPushFailure() {

                        }

                        @Override
                        public void onPushSuccess() {

                        }
                    }, true);

                    Log.v(TAG, "frameNumToSend : " + frameNumToSend);
                    Log.v(TAG, "completly finished in " + (new Date().getTime() - dateProcessBegin) + "ms - fail : " + failCount + " packet count : " + sendingNum);

                    if (progressListener != null) {
                        progressListener.onFinishUpload();
                    }
                }
            }
        } else {
            Log.v(TAG, "stop processing bitmap");
        }
    }

    private void sendBitmap(final byte[] bitmapData) {
        Log.v(TAG, "send bitmap with length : " + bitmapData.length);

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

        if (progressListener != null) {
            progressListener.onProgress(0);
        }

        conn.writeCharacteristic(SERVICE_BUTTON, TRANSMIT_STATUS, new byte[]{(byte) TransmitState.TRANSMIT_START.ordinal()}, new IPushListener() {
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
                        Log.v(TAG, "set bitmap length successfull");

                    }
                }, true);
                frameNumToSend = 127;
                if (!stopUpload) {
                    sendBitmapSequence();
                }
            }
        }, false);
    }

    @Override
    public void sendBitmapNoEncoding(final byte[] bitmapData) {
        sendBitmap(bitmapData);
    }

    @Override
    public void sendBitmapEncodedBitmask(final byte[] bitmapData, IProgressListener listener) {
        if (!mUploading) {
            this.progressListener = listener;
            stopUpload = false;
            mUploading = true;
            sendBitmap(bitmapData);
        } else {
            Log.e(TAG, "already uploading...");
        }
    }

    @Override
    public void cancelBitmap() {
        stopUpload = true;
        mUploading = false;
        conn.writeCharacteristic(SERVICE_BUTTON, TRANSMIT_STATUS, new byte[]{(byte) TransmitState.TRANSMIT_CANCEL.ordinal()}, new IPushListener() {
            @Override
            public void onPushFailure() {
                Log.e(TAG, "error happenend setting transmit status");
            }

            @Override
            public void onPushSuccess() {

            }
        }, false);
        if (progressListener != null) {
            progressListener.onFinishUpload();
            progressListener.onComplete();
        }
    }

    @Override
    public boolean isUploading() {
        return mUploading;
    }
}