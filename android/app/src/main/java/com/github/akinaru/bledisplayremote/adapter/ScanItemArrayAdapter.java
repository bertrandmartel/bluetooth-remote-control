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
package com.github.akinaru.bledisplayremote.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.github.akinaru.bledisplayremote.R;
import com.github.akinaru.bledisplayremote.bluetooth.events.BluetoothObject;
import com.github.akinaru.bledisplayremote.inter.IViewHolderClickListener;

import java.util.ArrayList;
import java.util.List;

/**
 * Adapter for Bluetooth scanned devices
 *
 * @author Bertrand Martel
 */
public class ScanItemArrayAdapter extends RecyclerView.Adapter<ScanItemArrayAdapter.ViewHolder> {

    /**
     * list of devices
     */
    private List<BluetoothObject> scanningList = new ArrayList<>();

    private Context mContext;

    private IViewHolderClickListener mListener;

    public ScanItemArrayAdapter(Context context, List<BluetoothObject> objects, IViewHolderClickListener listener) {
        this.mContext = context;
        this.scanningList = objects;
        this.mListener = listener;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View inflater = LayoutInflater.from(parent.getContext()).inflate(R.layout.listview_item, parent, false);
        return new ViewHolder(inflater, mListener);
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        BluetoothObject item = scanningList.get(position);

        holder.deviceName.setText(item.getDeviceName());
        holder.deviceAddress.setText(item.getDeviceAddress());
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public int getItemCount() {
        return scanningList.size();
    }

    /**
     * ViewHolder for HCI packet
     */
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

        public TextView deviceAddress;

        public TextView deviceName;

        public IViewHolderClickListener mListener;

        /**
         * ViewHolder
         *
         * @param v
         * @param listener
         */
        public ViewHolder(View v, IViewHolderClickListener listener) {
            super(v);
            mListener = listener;
            deviceAddress = (TextView) v.findViewById(R.id.address);
            deviceName = (TextView) v.findViewById(R.id.name);
            v.setOnClickListener(this);
        }

        @Override
        public void onClick(View v) {
            mListener.onClick(v);
        }
    }
}