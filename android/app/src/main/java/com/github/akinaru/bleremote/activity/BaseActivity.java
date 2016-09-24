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

package com.github.akinaru.bleremote.activity;

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
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.inter.IRemoteActivity;
import com.github.akinaru.bleremote.menu.MenuUtils;
import com.github.akinaru.bleremote.model.BitmapObj;
import com.github.akinaru.bleremote.service.BleDisplayRemoteService;
import com.github.akinaru.bleremote.utils.MediaStoreUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Abstract activity for all activities in Bluetooth LE Analyzer
 *
 * @author Bertrand Martel
 */
public abstract class BaseActivity extends AppCompatActivity implements IRemoteActivity {

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
     * bluetooth analyzer service
     */
    protected BleDisplayRemoteService mService = null;

    /**
     * activity layout ressource id
     */
    private int layoutId;

    /**
     * Bluetooth adapter
     */
    protected BluetoothAdapter mBluetoothAdapter = null;

    protected MenuItem deleteMenuItem;

    /**
     * set activity ressource id
     *
     * @param resId
     */
    protected void setLayout(int resId) {
        layoutId = resId;
    }

    public static int REQUEST_PICTURE = 4;
    public static int REQUEST_CROP_PICTURE = 5;

    protected boolean mExitOnBrowse = false;

    protected List<BitmapObj> mBitmapList = new ArrayList<>();
    protected List<BitmapObj> mDeleteBitmapList = new ArrayList<>();

    protected RecyclerView mBitmapRecyclerView;
    protected RecyclerView.Adapter mAdapter;

    protected File directory;

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
                        return false;
                    }
                });
    }

    /**
     * trigger a BLE scan
     */
    public void triggerNewScan() {

        if (mService != null && !mService.isScanning()) {
            mService.disconnectall();
            mService.startScan();
        }
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        this.getMenuInflater().inflate(R.menu.toolbar_menu, menu);

        //clear button
        MenuItem item = menu.findItem(R.id.add_image);
        if (item != null) {
            item.setOnMenuItemClickListener(new MenuItem.OnMenuItemClickListener() {

                @Override
                public boolean onMenuItemClick(MenuItem item) {
                    mExitOnBrowse = true;
                    startActivityForResult(MediaStoreUtils.getPickImageIntent(BaseActivity.this), REQUEST_PICTURE);
                    return true;
                }
            });
        }

        deleteMenuItem = menu.findItem(R.id.delete_image);

        if (deleteMenuItem != null) {

            deleteMenuItem.setOnMenuItemClickListener(new MenuItem.OnMenuItemClickListener() {

                @Override
                public boolean onMenuItemClick(MenuItem item) {
                    //delete
                    for (int i = 0; i < mDeleteBitmapList.size(); i++) {
                        mBitmapList.remove(mDeleteBitmapList.get(i));
                        File file = new File(directory, mDeleteBitmapList.get(i).getFileName());
                        file.delete();
                    }
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mBitmapRecyclerView.setAdapter(mAdapter);
                            mAdapter.notifyDataSetChanged();
                        }
                    });

                    return true;
                }
            });
        }

        return super.onCreateOptionsMenu(menu);
    }
}
