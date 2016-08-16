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

package com.github.akinaru.bleremote.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.inter.IViewHolderClickListener;
import com.github.akinaru.bleremote.inter.IViewHolderLongClickListener;
import com.github.akinaru.bleremote.model.BitmapObj;

import java.util.List;

/**
 * @author Bertrand Martel
 */
public class BitmapAdapter extends RecyclerView.Adapter<BitmapAdapter.ViewHolder> {

    private List<BitmapObj> mBitmapList;

    private IViewHolderClickListener mListener;

    private IViewHolderLongClickListener mLongClickListener;

    public BitmapAdapter(Context context, List<BitmapObj> bitmapList, IViewHolderClickListener listener, IViewHolderLongClickListener longClickListener) {
        mBitmapList = bitmapList;
        this.mListener = listener;
        this.mLongClickListener = longClickListener;
    }

    @Override
    public BitmapAdapter.ViewHolder onCreateViewHolder(ViewGroup parent,
                                                       int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.bitmap_card, parent, false);
        ViewHolder vh = new ViewHolder(v, mListener, mLongClickListener);
        return vh;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        BitmapObj item = mBitmapList.get(position);
        holder.bitmapObj = item;
        holder.bitmap.setImageBitmap(item.getBitmap());
    }

    @Override
    public int getItemCount() {
        return mBitmapList.size();
    }

    /**
     * ViewHolder for HCI packet
     */
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener, View.OnLongClickListener {

        public ImageView bitmap;

        public ImageView checkBitmap;

        public IViewHolderClickListener mListener;

        public IViewHolderLongClickListener mLongClickListener;

        public boolean mSelected;

        public BitmapObj bitmapObj;

        /**
         * ViewHolder
         *
         * @param v
         * @param listener
         */
        public ViewHolder(View v, IViewHolderClickListener listener, IViewHolderLongClickListener longClickListener) {
            super(v);
            mListener = listener;
            mLongClickListener = longClickListener;
            bitmap = (ImageView) v.findViewById(R.id.logo_image);
            checkBitmap = (ImageView) v.findViewById(R.id.check_bitmap);
            v.setOnClickListener(this);
            v.setOnLongClickListener(this);
        }

        @Override
        public void onClick(View v) {
            if (!mSelected) {
                mListener.onClick(v, bitmapObj);
            } else {
                bitmap.setAlpha(1f);
                checkBitmap.setVisibility(View.GONE);
                mSelected = false;
                mLongClickListener.onClick(v, bitmapObj, true);
            }
        }

        @Override
        public boolean onLongClick(View v) {
            if (!mSelected) {
                bitmap.setAlpha(0.1f);
                checkBitmap.setVisibility(View.VISIBLE);
                mSelected = true;
                mLongClickListener.onClick(v, bitmapObj, false);
            } else {
                bitmap.setAlpha(1f);
                checkBitmap.setVisibility(View.GONE);
                mSelected = false;
                mLongClickListener.onClick(v, bitmapObj, true);
            }
            return true;
        }
    }

}
