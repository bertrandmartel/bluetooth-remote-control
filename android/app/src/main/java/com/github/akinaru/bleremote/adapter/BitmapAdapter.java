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
import com.github.akinaru.bleremote.utils.MediaStoreUtils;

import java.util.List;

public class BitmapAdapter extends RecyclerView.Adapter<BitmapAdapter.ViewHolder> {

    private List<BitmapObj> mBitmapList;

    private IViewHolderClickListener mListener;

    private IViewHolderLongClickListener mLongClickListener;

    private Context mContext;

    // Provide a suitable constructor (depends on the kind of dataset)
    public BitmapAdapter(Context context, List<BitmapObj> bitmapList, IViewHolderClickListener listener, IViewHolderLongClickListener longClickListener) {
        mContext = context;
        mBitmapList = bitmapList;
        this.mListener = listener;
        this.mLongClickListener = longClickListener;
    }

    // Create new views (invoked by the layout manager)
    @Override
    public BitmapAdapter.ViewHolder onCreateViewHolder(ViewGroup parent,
                                                       int viewType) {
        // create a new view
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.bitmap_card, parent, false);
        // set the view's size, margins, paddings and layout parameters

        ViewHolder vh = new ViewHolder(v, mListener, mLongClickListener);
        return vh;
    }

    // Replace the contents of a view (invoked by the layout manager)
    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        BitmapObj item = mBitmapList.get(position);
        holder.bitmapObj = item;
        holder.bitmap.setImageBitmap(item.getBitmap());
        /*
        if (item.getRessourceId() == R.drawable.ic_control_point) {
            holder.bitmap.setPadding(0, 0, 0, 0);
            holder.bitmap.setScaleType(ImageView.ScaleType.CENTER);
        } else {
            holder.bitmap.setPadding(10, 0, 0, 0);
            holder.bitmap.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
        */
    }

    // Return the size of your dataset (invoked by the layout manager)
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
