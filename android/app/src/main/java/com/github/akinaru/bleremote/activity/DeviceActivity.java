package com.github.akinaru.bleremote.activity;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.adapter.BitmapAdapter;
import com.github.akinaru.bleremote.bluetooth.events.BluetoothEvents;
import com.github.akinaru.bleremote.bluetooth.events.BluetoothObject;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bleremote.inter.IBleDisplayRemoteDevice;
import com.github.akinaru.bleremote.inter.IButtonListener;
import com.github.akinaru.bleremote.inter.IDirectionPadListener;
import com.github.akinaru.bleremote.inter.IViewHolderClickListener;
import com.github.akinaru.bleremote.model.Button;
import com.github.akinaru.bleremote.model.ButtonState;
import com.github.akinaru.bleremote.model.DpadState;
import com.github.akinaru.bleremote.model.Led;
import com.github.akinaru.bleremote.service.BleDisplayRemoteService;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import uz.shift.colorpicker.LineColorPicker;
import uz.shift.colorpicker.OnColorChangedListener;

/**
 * Created by akinaru on 13/04/16.
 */
public class DeviceActivity extends BaseActivity {

    private final static String TAG = DeviceActivity.class.getSimpleName();

    private BluetoothObject mBtDevice = null;

    private IBleDisplayRemoteDevice mDisplayDevice;

    private HashMap<Button, ImageButton> buttonMap = new HashMap<>();

    private HashMap<Led, ImageButton> ledMap = new HashMap<>();

    private HashMap<DpadState, ImageButton> dpadMap = new HashMap<>();

    private byte mask = 0x00;

    private DpadState dpadState = DpadState.NONE;

    private List<Integer> mBitmapList = new ArrayList<>();

    private RecyclerView mBitmapRecyclerView;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager mLayoutManager;

    protected void onCreate(Bundle savedInstanceState) {
        setLayout(R.layout.device_activity);
        super.onCreate(savedInstanceState);

        mBitmapRecyclerView = (RecyclerView) findViewById(R.id.bitmap_recycler_view);

        // use this setting to improve performance if you know that changes
        // in content do not change the layout size of the RecyclerView
        mBitmapRecyclerView.setHasFixedSize(true);

        // use a linear layout manager
        mBitmapRecyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));

        mBitmapList.add(R.drawable.logo_github16);
        mBitmapList.add(R.drawable.ic_control_point);

        // specify an adapter (see also next example)
        mAdapter = new BitmapAdapter(this, mBitmapList, new IViewHolderClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "click");
            }
        });
        mBitmapRecyclerView.setAdapter(mAdapter);

        //register bluetooth event broadcast receiver
        registerReceiver(mBluetoothReceiver, makeGattUpdateIntentFilter());

        //set device address to analyze
        String deviceAddr = getIntent().getExtras().getString("deviceAddress");
        String deviceName = getIntent().getExtras().getString("deviceName");
        int adInterval = getIntent().getExtras().getInt("advertizingInterval");
        mBtDevice = new BluetoothObject(deviceAddr, deviceName, adInterval);

        RelativeLayout btnLayout = (RelativeLayout) findViewById(R.id.button_group);
        RelativeLayout ledLayout = (RelativeLayout) findViewById(R.id.led_group);

        buttonMap.put(Button.BUTTON1, (ImageButton) btnLayout.findViewById(R.id.button1));
        buttonMap.put(Button.BUTTON2, (ImageButton) btnLayout.findViewById(R.id.button2));
        buttonMap.put(Button.BUTTON3, (ImageButton) btnLayout.findViewById(R.id.button3));
        buttonMap.put(Button.BUTTON4, (ImageButton) btnLayout.findViewById(R.id.button4));

        buttonMap.get(Button.BUTTON1).setEnabled(false);
        buttonMap.get(Button.BUTTON2).setEnabled(false);
        buttonMap.get(Button.BUTTON3).setEnabled(false);
        buttonMap.get(Button.BUTTON4).setEnabled(false);

        ledMap.put(Led.LED1, (ImageButton) ledLayout.findViewById(R.id.led1));
        ledMap.put(Led.LED2, (ImageButton) ledLayout.findViewById(R.id.led2));
        ledMap.put(Led.LED3, (ImageButton) ledLayout.findViewById(R.id.led3));
        ledMap.put(Led.LED4, (ImageButton) ledLayout.findViewById(R.id.led4));

        dpadMap.put(DpadState.UP, (ImageButton) findViewById(R.id.nav_up));
        dpadMap.put(DpadState.DOWN, (ImageButton) findViewById(R.id.nav_down));
        dpadMap.put(DpadState.LEFT, (ImageButton) findViewById(R.id.nav_left));
        dpadMap.put(DpadState.RIGHT, (ImageButton) findViewById(R.id.nav_right));
        dpadMap.put(DpadState.SELECT, (ImageButton) findViewById(R.id.nav_ok));

        dpadMap.get(DpadState.UP).setEnabled(false);
        dpadMap.get(DpadState.DOWN).setEnabled(false);
        dpadMap.get(DpadState.LEFT).setEnabled(false);
        dpadMap.get(DpadState.RIGHT).setEnabled(false);
        dpadMap.get(DpadState.SELECT).setEnabled(false);

        ledMap.get(Led.LED1).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleLed(0, new IPushListener() {
                    @Override
                    public void onPushFailure() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(DeviceActivity.this, "fail to set led state", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }

                    @Override
                    public void onPushSuccess() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if ((mask & 0x01) == 0x01)
                                    ledMap.get(Led.LED1).setImageResource(R.drawable.nav_ok_normal_unique_green);
                                else
                                    ledMap.get(Led.LED1).setImageResource(R.drawable.nav_ok_pressed_unique);
                            }
                        });
                    }
                });
            }
        });

        ledMap.get(Led.LED2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleLed(1, new IPushListener() {
                    @Override
                    public void onPushFailure() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(DeviceActivity.this, "fail to set led state", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }

                    @Override
                    public void onPushSuccess() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if ((mask & 0x02) == 0x02)
                                    ledMap.get(Led.LED2).setImageResource(R.drawable.nav_ok_normal_unique_green);
                                else
                                    ledMap.get(Led.LED2).setImageResource(R.drawable.nav_ok_pressed_unique);
                            }
                        });
                    }
                });
            }
        });

        ledMap.get(Led.LED3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleLed(2, new IPushListener() {
                    @Override
                    public void onPushFailure() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(DeviceActivity.this, "fail to set led state", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }

                    @Override
                    public void onPushSuccess() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if ((mask & 0x04) == 0x04)
                                    ledMap.get(Led.LED3).setImageResource(R.drawable.nav_ok_normal_unique_green);
                                else
                                    ledMap.get(Led.LED3).setImageResource(R.drawable.nav_ok_pressed_unique);
                            }
                        });
                    }
                });
            }
        });

        ledMap.get(Led.LED4).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleLed(3, new IPushListener() {
                    @Override
                    public void onPushFailure() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(DeviceActivity.this, "fail to set led state", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }

                    @Override
                    public void onPushSuccess() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if ((mask & 0x08) == 0x08)
                                    ledMap.get(Led.LED4).setImageResource(R.drawable.nav_ok_normal_unique_green);
                                else
                                    ledMap.get(Led.LED4).setImageResource(R.drawable.nav_ok_pressed_unique);
                            }
                        });
                    }
                });
            }
        });

        LineColorPicker colorPicker = (LineColorPicker) findViewById(R.id.picker);

        String[] pallete = new String[]{"#28a5d6", "#a865ca", "#7bad02", "#fdbb33", "#d72b26", "#fe8801", "#9934c9", "#FFFFFF"};

        int[] colors = new int[pallete.length];

        for (int i = 0; i < colors.length; i++) {
            colors[i] = Color.parseColor(pallete[i]);
        }
        // set color palette
        colorPicker.setColors(colors);

        // set selected color [optional]
        colorPicker.setSelectedColor(Color.RED);

        // set on change listener
        colorPicker.setOnColorChangedListener(new OnColorChangedListener() {
            @Override
            public void onColorChanged(int color) {
                if (mDisplayDevice != null) {
                    mDisplayDevice.pushFullColor((byte) ((color >> 16) & 0xFF), (byte) ((color >> 8) & 0xFF), (byte) ((color >> 0) & 0xFF), new IPushListener() {
                        @Override
                        public void onPushFailure() {

                        }

                        @Override
                        public void onPushSuccess() {

                        }
                    });
                }
            }
        });

        //bind to service
        if (mBluetoothAdapter.isEnabled()) {
            Intent intent = new Intent(this, BleDisplayRemoteService.class);
            mBound = bindService(intent, mServiceConnection, BIND_AUTO_CREATE);
        }
    }

    private void toggleLed(int newMask, IPushListener listener) {
        if (mDisplayDevice != null) {
            mask ^= 1 << newMask;
            mDisplayDevice.pushLedState(mask, listener);
        } else {
            Log.e(TAG, "error display device null");
        }
    }

    /**
     * Manage Bluetooth Service
     */
    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {

            Log.v(TAG, "connected to service");

            mService = ((BleDisplayRemoteService.LocalBinder) service).getService();
            mService.connect(mBtDevice.getDeviceAddress());
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
        }
    };

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mBluetoothReceiver);
    }

    public static byte[] readFully(InputStream input) throws IOException {
        byte[] buffer = new byte[8192];
        int bytesRead;
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        while ((bytesRead = input.read(buffer)) != -1) {
            output.write(buffer, 0, bytesRead);
        }
        return output.toByteArray();
    }

    /**
     * broadcast receiver to receive bluetooth events
     */
    private final BroadcastReceiver mBluetoothReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            final String action = intent.getAction();

            if (BluetoothEvents.BT_EVENT_DEVICE_DISCONNECTED.equals(action)) {
                Log.v(TAG, "Device disconnected");
            } else if (BluetoothEvents.BT_EVENT_DEVICE_CONNECTED.equals(action)) {
                Log.v(TAG, "Device connected");

                if (mService.getConnectionList().get(mBtDevice.getDeviceAddress()) != null) {

                    if (mService.getConnectionList().get(mBtDevice.getDeviceAddress()).getDevice() instanceof IBleDisplayRemoteDevice) {

                        mDisplayDevice = (IBleDisplayRemoteDevice) mService.getConnectionList().get(mBtDevice.getDeviceAddress()).getDevice();

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ImageView button = (ImageView) findViewById(R.id.logo_image);
                                button.setOnClickListener(new View.OnClickListener() {
                                    @Override
                                    public void onClick(View v) {
                                        InputStream is = getResources().openRawResource(R.raw.github_reverse_compressed4096);
                                        try {
                                            mDisplayDevice.sendBitmapEncodedBitmask(readFully(is));
                                        } catch (IOException e) {
                                            e.printStackTrace();
                                        }
                                    }
                                });
                            }
                        });
                        mDisplayDevice.addButtonListener(new IButtonListener() {
                            @Override
                            public void onButtonStateChange(final Button button, ButtonState state) {
                                Log.i(TAG, "button state change : " + button.toString() + " | state : " + state.toString());
                                switch (state) {
                                    case PRESSED:
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                buttonMap.get(button).setPressed(true);
                                            }
                                        });
                                        break;
                                    case RELEASED:
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                buttonMap.get(button).setPressed(false);
                                            }
                                        });
                                        break;
                                }
                            }
                        });

                        mDisplayDevice.addDirectionPadListener(new IDirectionPadListener() {
                            @Override
                            public void onDPadStateChanged(DpadState state) {
                                Log.i(TAG, "dpad state change : " + state.toString());
                                switch (state) {
                                    case RIGHT:
                                        dpadState = DpadState.RIGHT;
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                dpadMap.get(DpadState.RIGHT).setPressed(true);
                                            }
                                        });
                                        break;
                                    case LEFT:
                                        dpadState = DpadState.LEFT;
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                dpadMap.get(DpadState.LEFT).setPressed(true);
                                            }
                                        });
                                        break;
                                    case UP:
                                        dpadState = DpadState.UP;
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                dpadMap.get(DpadState.UP).setPressed(true);
                                            }
                                        });
                                        break;
                                    case DOWN:
                                        dpadState = DpadState.DOWN;
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                dpadMap.get(DpadState.DOWN).setPressed(true);
                                            }
                                        });
                                        break;
                                    case SELECT:
                                        dpadState = DpadState.SELECT;
                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                dpadMap.get(DpadState.SELECT).setPressed(true);
                                            }
                                        });
                                        break;
                                    case NONE:
                                        switch (dpadState) {
                                            case UP:
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        dpadMap.get(DpadState.UP).setPressed(false);
                                                    }
                                                });
                                                break;
                                            case DOWN:
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        dpadMap.get(DpadState.DOWN).setPressed(false);
                                                    }
                                                });
                                                break;
                                            case LEFT:
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        dpadMap.get(DpadState.LEFT).setPressed(false);
                                                    }
                                                });
                                                break;
                                            case RIGHT:
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        dpadMap.get(DpadState.RIGHT).setPressed(false);
                                                    }
                                                });
                                                break;
                                            case SELECT:
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        dpadMap.get(DpadState.SELECT).setPressed(false);
                                                    }
                                                });
                                                break;
                                        }

                                        dpadState = DpadState.NONE;
                                        break;
                                }
                            }
                        });
                    }
                }
            }
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