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

import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.github.akinaru.bledisplayremote.R;
import com.github.akinaru.bledisplayremote.inter.IBtActivity;
import com.github.akinaru.bledisplayremote.menu.MenuUtils;
import com.github.akinaru.bledisplayremote.service.BleDisplayRemoteService;

/**
 * Abstract activity for all activities in Bluetooth LE Analyzer
 *
 * @author Bertrand Martel
 */
public abstract class BaseActivity extends AppCompatActivity implements IBtActivity {

    private final static String TAG = BaseActivity.class.getSimpleName();

    /**
     * define is service is bound or not
     */
    protected boolean mBound = false;

    /**
     * application toolbar
     */
    protected Toolbar toolbar = null;

    /**
     * navigationdrawer
     */
    protected DrawerLayout mDrawer = null;

    /**
     * toggle on the hambureger button
     */
    protected ActionBarDrawerToggle drawerToggle;

    /**
     * navigation view
     */
    protected NavigationView nvDrawer;

    /**
     * BlBBBdfbdfbdqsdqsd
     * bluetooth analyzer service
     */
    protected BleDisplayRemoteService mService = null;

    /**
     * define if bluetooth is enabled on device
     */
    protected final static int REQUEST_ENABLE_BT = 1;

    /**
     * activity layout ressource id
     */
    private int layoutId;

    /**
     * Bluetooth adapter
     */
    protected BluetoothAdapter mBluetoothAdapter = null;

    /**
     * set activity ressource id
     *
     * @param resId
     */
    protected void setLayout(int resId) {
        layoutId = resId;
    }

    /**
     * scan image button at top right
     */
    protected ImageButton scanImage;

    /**
     * toolbar progress bar at top right
     */
    protected ProgressBar progressBar;

    protected MenuItem scanMenuItem;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(layoutId);

        if (Build.VERSION.SDK_INT >= 21) {
            Window window = getWindow();
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.setStatusBarColor(getResources().getColor(R.color.colorPrimaryDark));
        }
        // Set a Toolbar to replace the ActionBar.
        toolbar = (Toolbar) findViewById(R.id.toolbar_item);
        setSupportActionBar(toolbar);
        getSupportActionBar().setTitle(getResources().getString(R.string.bt_device_title));
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        toolbar.inflateMenu(R.menu.toolbar_menu);

        // Find our drawer view
        mDrawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawerToggle = setupDrawerToggle();
        mDrawer.setDrawerListener(drawerToggle);
        nvDrawer = (NavigationView) findViewById(R.id.nvView);

        // Setup drawer view
        setupDrawerContent(nvDrawer);

        //setup bluetooth
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, getResources().getString(R.string.ble_not_supported), Toast.LENGTH_SHORT).show();
            finish();
        }

        //setup bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }

    /**
     * setup navigation view
     *
     * @param navigationView
     */
    private void setupDrawerContent(NavigationView navigationView) {

        navigationView.setNavigationItemSelectedListener(
                new NavigationView.OnNavigationItemSelectedListener() {
                    @Override
                    public boolean onNavigationItemSelected(MenuItem menuItem) {
                        MenuUtils.selectDrawerItem(menuItem, mDrawer, BaseActivity.this, BaseActivity.this);
                        return true;
                    }
                });
    }

    /**
     * setup action drawer
     *
     * @return
     */
    protected ActionBarDrawerToggle setupDrawerToggle() {
        return new ActionBarDrawerToggle(this, mDrawer, toolbar, R.string.drawer_open, R.string.drawer_close);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                mDrawer.openDrawer(GravityCompat.START);
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    // Make sure this is the method with just `Bundle` as the signature
    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        drawerToggle.syncState();
    }

    @Override
    public void onBackPressed() {
        if (this.mDrawer.isDrawerOpen(GravityCompat.START)) {
            this.mDrawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    /**
     * toggle bluetooth scanning state
     */
    @Override
    public void toggleScan() {

        if (mService != null && mService.isScanning()) {
            Log.v(TAG, "scanning stopped...");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(BaseActivity.this, getResources().getString(R.string.toast_scan_stop), Toast.LENGTH_SHORT).show();
                    nvDrawer.getMenu().findItem(R.id.scan_btn_nv).setIcon(R.drawable.ic_looks);
                    nvDrawer.getMenu().findItem(R.id.scan_btn_nv).setTitle(getResources().getString(R.string.menu_title_start_scan));
                }
            });
            hideProgressBar();
            mService.stopScan();
        } else {
            Log.v(TAG, "scanning ...");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(BaseActivity.this, getResources().getString(R.string.toast_scan_start), Toast.LENGTH_SHORT).show();
                    nvDrawer.getMenu().findItem(R.id.scan_btn_nv).setIcon(R.drawable.ic_portable_wifi_off);
                    nvDrawer.getMenu().findItem(R.id.scan_btn_nv).setTitle(getResources().getString(R.string.menu_title_stop_scan));
                }
            });
            triggerNewScan();
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
    public boolean onCreateOptionsMenu(Menu menu) {
        this.getMenuInflater().inflate(R.menu.toolbar_menu, menu);
        scanMenuItem = menu.findItem(R.id.scanning_button);
        scanImage = (ImageButton) menu.findItem(R.id.scanning_button).getActionView().findViewById(R.id.bluetooth_scan_stop);
        progressBar = (ProgressBar) menu.findItem(R.id.scanning_button).getActionView().findViewById(R.id.bluetooth_scanning);
        scanImage.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                toggleScan();
            }
        });
        progressBar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleScan();
            }
        });
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                showProgressBar();
            }
        });
        return super.onCreateOptionsMenu(menu);
    }

    /**
     * show progress bar to indicate scanning
     */
    protected void showProgressBar() {
        if (scanImage != null)
            scanImage.setVisibility(View.GONE);
        if (progressBar != null)
            progressBar.setVisibility(View.VISIBLE);
    }

    /**
     * hide scanning progress bar
     */
    protected void hideProgressBar() {
        if (scanImage != null)
            scanImage.setVisibility(View.VISIBLE);
        if (progressBar != null)
            progressBar.setVisibility(View.GONE);
    }
}
