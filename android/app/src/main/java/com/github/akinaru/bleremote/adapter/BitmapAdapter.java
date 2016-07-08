package com.github.akinaru.bleremote.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.github.akinaru.bleremote.R;
import com.github.akinaru.bleremote.inter.IViewHolderClickListener;
import com.github.akinaru.bleremote.model.BitmapObj;

import java.util.List;

public class BitmapAdapter extends RecyclerView.Adapter<BitmapAdapter.ViewHolder> {

    private List<BitmapObj> mBitmapList;

    private IViewHolderClickListener mListener;

    private Context mContext;

    // Provide a suitable constructor (depends on the kind of dataset)
    public BitmapAdapter(Context context, List<BitmapObj> bitmapList, IViewHolderClickListener listener) {
        mContext = context;
        mBitmapList = bitmapList;
        this.mListener = listener;
    }

    // Create new views (invoked by the layout manager)
    @Override
    public BitmapAdapter.ViewHolder onCreateViewHolder(ViewGroup parent,
                                                       int viewType) {
        // create a new view
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.bitmap_card, parent, false);
        // set the view's size, margins, paddings and layout parameters

        ViewHolder vh = new ViewHolder(v, mListener);
        return vh;
    }

    // Replace the contents of a view (invoked by the layout manager)
    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        BitmapObj item = mBitmapList.get(position);
        holder.ressourceId = item.getRessourceId();
        holder.bitmap.setImageBitmap(item.getBitmap());
        if (item.getRessourceId() == R.drawable.ic_control_point) {
            holder.bitmap.setPadding(0, 0, 0, 0);
            holder.bitmap.setScaleType(ImageView.ScaleType.CENTER);
        } else {
            holder.bitmap.setPadding(10, 0, 0, 0);
            holder.bitmap.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }

    // Return the size of your dataset (invoked by the layout manager)
    @Override
    public int getItemCount() {
        return mBitmapList.size();
    }

    /**
     * ViewHolder for HCI packet
     */
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

        public ImageView bitmap;

        public IViewHolderClickListener mListener;

        public int ressourceId;

        /**
         * ViewHolder
         *
         * @param v
         * @param listener
         */
        public ViewHolder(View v, IViewHolderClickListener listener) {
            super(v);
            mListener = listener;
            bitmap = (ImageView) v.findViewById(R.id.logo_image);
            v.setOnClickListener(this);
        }

        @Override
        public void onClick(View v) {
            mListener.onClick(v, ressourceId);
        }
    }
}
