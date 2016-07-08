package com.github.akinaru.bleremote.activity;

import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.OpenableColumns;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
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
import com.github.akinaru.bleremote.model.BitmapObj;
import com.github.akinaru.bleremote.model.Button;
import com.github.akinaru.bleremote.model.ButtonState;
import com.github.akinaru.bleremote.model.DpadState;
import com.github.akinaru.bleremote.model.Led;
import com.github.akinaru.bleremote.service.BleDisplayRemoteService;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;
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

    private List<BitmapObj> mBitmapList = new ArrayList<>();

    private RecyclerView mBitmapRecyclerView;
    private RecyclerView.Adapter mAdapter;

    private ProgressDialog mConnectingProgressDialog;

    private boolean mExitOnBrowse = false;

    private final static String BITMAP_DIRECTORY = "bitmap";

    final int PIC_CROP = 3;

    protected void onCreate(Bundle savedInstanceState) {
        setLayout(R.layout.device_activity);
        super.onCreate(savedInstanceState);

        mBitmapRecyclerView = (RecyclerView) findViewById(R.id.bitmap_recycler_view);

        // use this setting to improve performance if you know that changes
        // in content do not change the layout size of the RecyclerView
        mBitmapRecyclerView.setHasFixedSize(true);

        // use a linear layout manager
        mBitmapRecyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));

        //mBitmapList.add(R.drawable.logo_github16);

        ContextWrapper cw = new ContextWrapper(getApplicationContext());
        File directory = cw.getDir(BITMAP_DIRECTORY, Context.MODE_PRIVATE);

        Log.i(TAG, "directory : " + directory.exists());

        int size = loadImageFromStorage(directory.getPath());

        if (size == 0) {
            Log.i(TAG, "saving default image");
            Bitmap bm = BitmapFactory.decodeResource(getResources(), R.drawable.logo_github16);
            try {
                saveToInternalStorage(bm, "github.bmp", BITMAP_DIRECTORY);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        mBitmapList.add(new BitmapObj(BitmapFactory.decodeResource(getResources(), R.drawable.ic_control_point), R.drawable.ic_control_point));

        // specify an adapter (see also next example)
        mAdapter = new BitmapAdapter(this, mBitmapList, new IViewHolderClickListener() {
            @Override
            public void onClick(View v, int ressourceId) {

                if (ressourceId == R.drawable.ic_control_point) {
                    mExitOnBrowse = true;
                    showFileChooser();
                } else {
                    /*
                    InputStream is = getResources().openRawResource(R.raw.github_reverse_compressed4096);
                    try {
                        mDisplayDevice.sendBitmapEncodedBitmask(readFully(is));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    */
                }
            }
        });
        mBitmapRecyclerView.setAdapter(mAdapter);

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
        //register bluetooth event broadcast receiver
        registerReceiver(mBluetoothReceiver, makeGattUpdateIntentFilter());

        //bind to service
        if (mBluetoothAdapter.isEnabled()) {
            Intent intent = new Intent(this, BleDisplayRemoteService.class);
            mBound = bindService(intent, mServiceConnection, BIND_AUTO_CREATE);
        }
    }

    private void performCrop(Uri picUri) {
        try {

            Intent cropIntent = new Intent("com.android.camera.action.CROP");
            // indicate image type and Uri
            cropIntent.setDataAndType(picUri, "image/*");
            // set crop properties
            cropIntent.putExtra("crop", "true");
            // indicate aspect of desired crop
            cropIntent.putExtra("aspectX", 1);
            cropIntent.putExtra("aspectY", 1);
            // indicate output X and Y
            cropIntent.putExtra("outputX", 128);
            cropIntent.putExtra("outputY", 128);
            // retrieve data on return
            cropIntent.putExtra("return-data", true);
            // start the activity - we handle returning in onActivityResult
            startActivityForResult(cropIntent, PIC_CROP);
        }
        // respond to users whose devices do not support the crop action
        catch (ActivityNotFoundException anfe) {
            // display an error message
            String errorMessage = "Whoops - your device doesn't support the crop action!";
            Toast toast = Toast.makeText(this, errorMessage, Toast.LENGTH_SHORT);
            toast.show();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mExitOnBrowse = false;
        createProgressConnect();
        triggerNewScan();
    }


    @Override
    protected void onPause() {
        super.onPause();

        if (!mExitOnBrowse) {
            if (mService != null) {
                mService.disconnectall();
            }
            if (mConnectingProgressDialog != null) {
                mConnectingProgressDialog.cancel();
                mConnectingProgressDialog.dismiss();
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mBluetoothReceiver);
        unbindService(mServiceConnection);
        mBound = false;
    }

    private void createProgressConnect() {
        if (mConnectingProgressDialog != null) {
            mConnectingProgressDialog.cancel();
            mConnectingProgressDialog.dismiss();
        }
        mConnectingProgressDialog = ProgressDialog.show(this, "", "Looking for for device ...");
        mConnectingProgressDialog.show();
    }

    private void toggleLed(int newMask, IPushListener listener) {
        if (mDisplayDevice != null) {
            mask ^= 1 << newMask;
            mDisplayDevice.pushLedState(mask, listener);
        } else {
            Log.e(TAG, "error display device null");
        }
    }

    private static final int FILE_SELECT_CODE = 0;

    private void showFileChooser() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);

        try {
            startActivityForResult(
                    Intent.createChooser(intent, "Select a File to Upload"),
                    FILE_SELECT_CODE);
        } catch (android.content.ActivityNotFoundException ex) {
            // Potentially direct the user to the Market with a Dialog
            Toast.makeText(this, "Please install a File Manager.",
                    Toast.LENGTH_SHORT).show();
        }
    }

    public static String getPath(Context context, Uri uri) throws URISyntaxException {
        if ("content".equalsIgnoreCase(uri.getScheme())) {
            String[] projection = {"_data"};
            Cursor cursor = null;

            try {
                cursor = context.getContentResolver().query(uri, projection, null, null, null);
                int column_index = cursor.getColumnIndexOrThrow("_data");
                if (cursor.moveToFirst()) {
                    return cursor.getString(column_index);
                }
            } catch (Exception e) {
                // Eat it
            }
        } else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
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
        } else if (requestCode == FILE_SELECT_CODE) {

            if (resultCode == RESULT_OK) {
                // Get the Uri of the selected file
                Uri uri = data.getData();
                Log.d(TAG, "File Uri: " + uri.toString());
                performCrop(uri);
                /*
                try {
                    InputStream input = getContentResolver().openInputStream(uri);
                    Bitmap bmp = BitmapFactory.decodeStream(input);

                    bmp = Bitmap.createScaledBitmap(bmp, 128, 160, true);
                    Log.i(TAG, "bmp size : " + bmp.getByteCount());
                    int bytes = bmp.getByteCount();
                    ByteBuffer buffer = ByteBuffer.allocate(bytes); //Create a new buffer
                    bmp.copyPixelsToBuffer(buffer); //Move the byte data to the buffer

                    byte[] bitmapData = new byte[128 * 160 * 2];

                    switch (bmp.getConfig()) {
                        case RGB_565:
                            break;
                        case ARGB_8888:
                            byte[] bitmapOldData = buffer.array();
                            int index = 0;
                            for (int i = 0; i < bitmapOldData.length; i++) {
                                i++;
                                bitmapData[index++] = (byte) (((bitmapOldData[i + 1] & 0x1F) << 3) + ((bitmapOldData[i + 2] & 0x3F) >> 3));
                                bitmapData[index++] = (byte) (((bitmapOldData[i + 2] & 0x3F) << 5) + ((bitmapOldData[i + 3] & 0x1F)));
                                i += 3;
                            }
                            Log.i(TAG, "new size : " + bitmapData.length);
                            break;
                        case ARGB_4444:
                            break;
                        case ALPHA_8:
                            break;
                    }

                    Log.i(TAG, "saving imported image");

                    saveToInternalStorage(bmp, getFileName(uri), BITMAP_DIRECTORY);

                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                */
            }
        } else if (requestCode == PIC_CROP) {
            if (data != null) {
                // get the returned data
                Bundle extras = data.getExtras();
                // get the cropped bitmap
                Bitmap selectedBitmap = extras.getParcelable("data");

                //imgView.setImageBitmap(selectedBitmap);
            }
        }
    }

    public String getFileName(Uri uri) {
        String result = null;
        if (uri.getScheme().equals("content")) {
            Cursor cursor = getContentResolver().query(uri, null, null, null, null);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    result = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
                }
            } finally {
                cursor.close();
            }
        }
        if (result == null) {
            result = uri.getPath();
            int cut = result.lastIndexOf('/');
            if (cut != -1) {
                result = result.substring(cut + 1);
            }
        }
        return result;
    }

    private String saveToInternalStorage(Bitmap bitmapImage, String fileName, String directoryName) throws IOException {
        ContextWrapper cw = new ContextWrapper(getApplicationContext());
        // path to /data/data/yourapp/app_data/imageDir
        File directory = cw.getDir(directoryName, Context.MODE_PRIVATE);
        // Create imageDir
        File mypath = new File(directory, fileName);

        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(mypath);
            // Use the compress method on the BitMap object to write image to the OutputStream
            bitmapImage.compress(Bitmap.CompressFormat.PNG, 100, fos);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            fos.close();
        }
        return directory.getAbsolutePath();
    }

    private int loadImageFromStorage(String path) {
        try {
            File f = new File(path);
            File file[] = f.listFiles();
            Log.d("Files", "Size: " + file.length);
            for (int i = 0; i < file.length; i++) {
                Log.d("Files", "FileName:" + file[i].getName());
                Bitmap b = BitmapFactory.decodeStream(new FileInputStream(file[i]));
                mBitmapList.add(new BitmapObj(b, 0));
            }
            return file.length;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return 0;
    }

    public byte[] readBytes(InputStream inputStream) throws IOException {
        // this dynamically extends to take the bytes you read
        ByteArrayOutputStream byteBuffer = new ByteArrayOutputStream();

        // this is storage overwritten on each iteration with bytes
        int bufferSize = 1024;
        byte[] buffer = new byte[bufferSize];

        // we need to know how may bytes were read to write them to the byteBuffer
        int len = 0;
        while ((len = inputStream.read(buffer)) != -1) {
            byteBuffer.write(buffer, 0, len);
        }

        // and then we can return your byte array.
        return byteBuffer.toByteArray();
    }

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
                    triggerNewScan();
                }
            });
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
        }
    };

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
                createProgressConnect();
                mService.clearScanningList();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        triggerNewScan();
                    }
                });
            } else if (BluetoothEvents.BT_EVENT_DEVICE_CONNECTED.equals(action)) {
                Log.v(TAG, "Device connected");
                Toast.makeText(DeviceActivity.this, "device connected", Toast.LENGTH_SHORT).show();
                mConnectingProgressDialog.cancel();
                mConnectingProgressDialog.dismiss();
                if (mService.getConnectionList().get(mBtDevice.getDeviceAddress()) != null) {

                    if (mService.getConnectionList().get(mBtDevice.getDeviceAddress()).getDevice() instanceof IBleDisplayRemoteDevice) {

                        mDisplayDevice = (IBleDisplayRemoteDevice) mService.getConnectionList().get(mBtDevice.getDeviceAddress()).getDevice();

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
            } else if (BluetoothEvents.BT_EVENT_SCAN_START.equals(action)) {
                Log.v(TAG, "Scan has started");
            } else if (BluetoothEvents.BT_EVENT_SCAN_END.equals(action)) {
                Log.v(TAG, "Scan has ended");
            } else if (BluetoothEvents.BT_EVENT_DEVICE_DISCOVERED.equals(action)) {
                Log.v(TAG, "New device has been discovered");
                final BluetoothObject btDeviceTmp = BluetoothObject.parseArrayList(intent);

                if (btDeviceTmp.getDeviceName().equals("BleDisplayRemote")) {
                    Log.v(TAG, "found new device");
                    mConnectingProgressDialog.setMessage("Connecting to device ...");
                    mService.stopScan();
                    mBtDevice = btDeviceTmp;
                    mService.connect(mBtDevice.getDeviceAddress());
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

    @Override
    public void disconnect() {
        if (mService != null) {
            mService.disconnectall();
        }
    }
}