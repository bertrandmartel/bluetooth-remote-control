package com.github.akinaru.bleremote.activity;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.android.camera.CropImageIntentBuilder;
import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.adapter.BitmapAdapter;
import com.github.akinaru.bleremote.bluetooth.events.BluetoothEvents;
import com.github.akinaru.bleremote.bluetooth.events.BluetoothObject;
import com.github.akinaru.bleremote.bluetooth.listener.IPushListener;
import com.github.akinaru.bleremote.inter.IBleDisplayRemoteDevice;
import com.github.akinaru.bleremote.inter.IButtonListener;
import com.github.akinaru.bleremote.inter.IDirectionPadListener;
import com.github.akinaru.bleremote.inter.IProgressListener;
import com.github.akinaru.bleremote.inter.IViewHolderClickListener;
import com.github.akinaru.bleremote.inter.IViewHolderLongClickListener;
import com.github.akinaru.bleremote.model.BitmapObj;
import com.github.akinaru.bleremote.model.Button;
import com.github.akinaru.bleremote.model.ButtonState;
import com.github.akinaru.bleremote.model.DpadState;
import com.github.akinaru.bleremote.model.Led;
import com.github.akinaru.bleremote.service.BleDisplayRemoteService;
import com.github.akinaru.bleremote.utils.RandomGen;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

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

    private ProgressDialog mConnectingProgressDialog;

    private final static String BITMAP_DIRECTORY = "bitmap";

    private final static String TMP_DIRECTORY = "tmp";

    private String mSavedImage;

    private ProgressBar mProgressBar;
    private Handler mHandler = new Handler();
    private TextView mProgressionTv;
    private android.widget.Button mButton;

    private TextView mCompleteTv;

    private int mTimeCount;
    private Timer mTimer;

    private Dialog mProgressDialog;

    protected Bitmap flip(Bitmap d) {
        Matrix m = new Matrix();
        m.preScale(-1, 1);
        Bitmap dst = Bitmap.createBitmap(d, 0, 0, d.getWidth(), d.getHeight(), m, false);
        dst.setDensity(DisplayMetrics.DENSITY_DEFAULT);
        return dst;
    }

    private void generateDialog(int uncompressedSize, int compressedSize) {

        mProgressDialog = new Dialog(this);
        mProgressDialog.setContentView(R.layout.progress);
        mProgressDialog.setTitle("");

        mProgressBar = (ProgressBar) mProgressDialog.findViewById(R.id.progressbar);
        mProgressionTv = (TextView) mProgressDialog.findViewById(R.id.progression);

        altTableRow(2, (TableLayout) mProgressDialog.findViewById(R.id.tablelayout));

        TextView text = (TextView) mProgressDialog.findViewById(R.id.uncompressed_size_value);
        text.setText(uncompressedSize + " octets");
        TextView text1 = (TextView) mProgressDialog.findViewById(R.id.size_compressed_value);
        text1.setText(compressedSize + " octets");

        mCompleteTv = (TextView) mProgressDialog.findViewById(R.id.unpack_time_value);

        final TextView text3 = (TextView) mProgressDialog.findViewById(R.id.upload_time_value);
        if (uncompressedSize != 0) {
            TextView text2 = (TextView) mProgressDialog.findViewById(R.id.compression_rate_value);
            text2.setText((100 - (compressedSize * 100) / uncompressedSize) + " %");
        }

        mButton = (android.widget.Button) mProgressDialog.findViewById(R.id.cancel_button);
        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mProgressDialog != null) {
                    mProgressDialog.dismiss();
                }
                if (mDisplayDevice != null) {
                    mDisplayDevice.cancelBitmap();
                }
            }
        });

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mProgressDialog.show();
                WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
                Window window = mProgressDialog.getWindow();
                lp.copyFrom(window.getAttributes());
                lp.width = WindowManager.LayoutParams.MATCH_PARENT;
                lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
                window.setAttributes(lp);
            }
        });

        mTimeCount = 0;
        if (mTimer != null) {
            mTimer.cancel();
            mTimer.purge();
        }
        mTimer = new Timer();
        mTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        text3.setText(formatSeconds(mTimeCount));
                        mTimeCount++;
                    }
                });
            }
        }, 0, 1000);
    }

    private String formatSeconds(int seconds) {
        Date d = new Date(seconds * 1000L);
        SimpleDateFormat df = new SimpleDateFormat("HH:mm:ss"); // HH for 0-23
        df.setTimeZone(TimeZone.getTimeZone("GMT"));
        return df.format(d);
    }

    /**
     * alternate colors for description rows
     *
     * @param alt_row
     */
    public void altTableRow(int alt_row, TableLayout tablelayout) {

        int childViewCount = tablelayout.getChildCount();

        for (int i = 0; i < childViewCount; i++) {
            TableRow row = (TableRow) tablelayout.getChildAt(i);

            for (int j = 0; j < row.getChildCount(); j++) {

                TextView tv = (TextView) row.getChildAt(j);
                if (i % alt_row != 0) {
                    tv.setBackground(getResources().getDrawable(
                            R.drawable.alt_row_color));
                } else {
                    tv.setBackground(getResources().getDrawable(
                            R.drawable.row_color));
                }
            }
        }
    }

    protected void onCreate(Bundle savedInstanceState) {
        setLayout(R.layout.device_activity);
        super.onCreate(savedInstanceState);

        mBitmapRecyclerView = (RecyclerView) findViewById(R.id.bitmap_recycler_view);

        // use this setting to improve performance if you know that changes
        // in content do not change the layout size of the RecyclerView
        mBitmapRecyclerView.setHasFixedSize(true);

        // use a linear layout manager
        mBitmapRecyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));

        ContextWrapper cw = new ContextWrapper(getApplicationContext());
        directory = cw.getDir(BITMAP_DIRECTORY, Context.MODE_PRIVATE);

        Log.i(TAG, "directory : " + directory.exists());

        int size = loadImageFromStorage(directory.getPath());

        if (size == 0) {
            Log.i(TAG, "saving default image");
            Bitmap bm = BitmapFactory.decodeResource(getResources(), R.raw.logo_github_default);
            bm = Bitmap.createScaledBitmap(bm, 128, 160, false);

            int bytes = bm.getByteCount();
            Log.i(TAG, "byte count => " + bytes);

            ByteBuffer buffer = ByteBuffer.allocate(bytes);
            bm.copyPixelsToBuffer(buffer);

            try {
                saveToInternalStorage(bm, "github.bmp", BITMAP_DIRECTORY);
            } catch (IOException e) {
                e.printStackTrace();
            }
            mBitmapList.add(new BitmapObj(bm, "github.bmp"));
        }

        // specify an adapter (see also next example)
        mAdapter = new BitmapAdapter(this, mBitmapList, new IViewHolderClickListener() {

            @Override
            public void onClick(View v, BitmapObj bm) {

                Bitmap bmp = bm.getBitmap();
                int bytes = bmp.getByteCount();
                ByteBuffer buffer = ByteBuffer.allocate(bytes);
                bmp.copyPixelsToBuffer(buffer);
                byte[] array = buffer.array();

                byte[] bitmapData = new byte[128 * 160 * 2];

                switch (bmp.getConfig()) {
                    case RGB_565:
                        bitmapData = buffer.array();
                        break;
                    case ARGB_8888:
                        Bitmap converted = bmp.copy(Bitmap.Config.RGB_565, false);
                        converted = flip(converted);
                        ByteBuffer buffer2 = ByteBuffer.allocate(converted.getByteCount());
                        converted.copyPixelsToBuffer(buffer2);
                        bitmapData = buffer2.array();
                        break;
                    case ARGB_4444:
                        break;
                    case ALPHA_8:
                        break;
                }

                String inputPath = "";

                ContextWrapper cw = new ContextWrapper(getApplicationContext());
                File directory = cw.getDir(TMP_DIRECTORY, Context.MODE_PRIVATE);
                File outputFile = new File(directory, "tmp_output_file");
                if (outputFile.exists()) {
                    outputFile.delete();
                }

                try {
                    //inputPath = saveToInternalStorage(bitmapData, "tmp_input_file", TMP_DIRECTORY);
                    inputPath = saveToInternalStorage(bitmapData, "tmp_input_file", TMP_DIRECTORY);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                Log.i(TAG, "from path : " + inputPath + " to " + outputFile.getAbsolutePath());

                int ret = BleDisplayRemoteService.pack(inputPath, outputFile.getAbsolutePath());

                Log.i(TAG, "fastlz pack result : " + ret);

                File file = new File(outputFile.getAbsolutePath());
                byte[] data = new byte[(int) file.length()];
                try {
                    new FileInputStream(file).read(data);
                } catch (Exception e) {
                    e.printStackTrace();
                }

                Log.i(TAG, "data.pack => " + data.length + " for " + file.length());

                generateDialog(bitmapData.length, data.length);

                mDisplayDevice.sendBitmapEncodedBitmask(data, new IProgressListener() {
                    @Override
                    public void onProgress(final int progress) {
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                if (mProgressionTv != null && mProgressBar != null) {
                                    mProgressionTv.setText(progress + " %");
                                    mProgressBar.setProgress(progress);
                                }
                            }
                        });
                    }

                    @Override
                    public void onFinishUpload() {
                        if (mButton != null) {
                            if (mTimer != null) {
                                mTimer.cancel();
                                mTimer.purge();
                            }
                            mTimeCount = 0;

                            mTimer = new Timer();
                            mTimer.scheduleAtFixedRate(new TimerTask() {
                                @Override
                                public void run() {
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            if (mCompleteTv != null) {
                                                mCompleteTv.setText(formatSeconds(mTimeCount));
                                            }
                                            mTimeCount++;
                                        }
                                    });
                                }
                            }, 0, 1000);
                        }
                    }

                    @Override
                    public void onComplete() {
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mButton.setText("OK");
                            }
                        });
                        if (mTimer != null) {
                            mTimer.cancel();
                            mTimer.purge();
                        }
                        mTimeCount = 0;
                    }
                });

            }
        }, new IViewHolderLongClickListener() {
            @Override
            public void onClick(View v, BitmapObj bm, boolean remove) {
                if (!remove) {
                    mDeleteBitmapList.add(bm);
                    deleteMenuItem.setVisible(true);
                } else {
                    mDeleteBitmapList.remove(bm);
                    if (mDeleteBitmapList.size() == 0) {
                        deleteMenuItem.setVisible(false);
                    }
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


    @Override
    protected void onResume() {
        super.onResume();
        if (!mExitOnBrowse) {
            createProgressConnect();
            triggerNewScan();
        }
        mExitOnBrowse = false;
    }


    @Override
    protected void onPause() {
        super.onPause();

        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
        }

        if (mTimer != null) {
            mTimer.cancel();
            mTimer.purge();
        }
        mTimeCount = 0;

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
        } else if ((requestCode == REQUEST_PICTURE) && (resultCode == RESULT_OK)) {

            mSavedImage = new RandomGen(20).nextString();

            File croppedImageFile = new File(getFilesDir(), mSavedImage + ".jpg");

            // When the user is done picking a picture, let's start the CropImage Activity,
            // setting the output image file and size to 200x200 pixels square.
            Uri croppedImage = Uri.fromFile(croppedImageFile);

            CropImageIntentBuilder cropImage = new CropImageIntentBuilder(128, 160, croppedImage);
            cropImage.setOutlineColor(0xFF03A9F4);
            cropImage.setSourceImage(data.getData());

            startActivityForResult(cropImage.getIntent(this), REQUEST_CROP_PICTURE);
        } else if ((requestCode == REQUEST_CROP_PICTURE) && (resultCode == RESULT_OK)) {

            File croppedImageFile = new File(getFilesDir(), mSavedImage + ".jpg");

            Bitmap bm = BitmapFactory.decodeFile(croppedImageFile.getAbsolutePath());

            try {
                saveToInternalStorage(bm, croppedImageFile.getName(), BITMAP_DIRECTORY);
            } catch (IOException e) {
                e.printStackTrace();
            }

            mBitmapList.add(new BitmapObj(bm, mSavedImage + ".jpg"));
            mAdapter.notifyDataSetChanged();
        }
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

    private String saveToInternalStorage(byte[] data, String fileName, String directoryName) throws IOException {
        ContextWrapper cw = new ContextWrapper(getApplicationContext());
        // path to /data/data/yourapp/app_data/imageDir
        File directory = cw.getDir(directoryName, Context.MODE_PRIVATE);
        // Create imageDir
        File mypath = new File(directory, fileName);
        if (mypath.exists()) {
            mypath.delete();
        }
        BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(mypath));
        bos.write(data);
        bos.flush();
        bos.close();

        return mypath.getAbsolutePath();
    }

    private int loadImageFromStorage(String path) {
        try {
            File f = new File(path);
            File file[] = f.listFiles();
            Log.d("Files", "Size: " + file.length);
            for (int i = 0; i < file.length; i++) {
                Log.d("Files", "FileName:" + file[i].getName());
                Bitmap b = BitmapFactory.decodeStream(new FileInputStream(file[i]));
                mBitmapList.add(new BitmapObj(b, file[i].getName()));
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
                /*
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        triggerNewScan();
                    }
                });
                */
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