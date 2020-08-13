package com.android.ffmpegturorial;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Description:
 *
 * @author wangzhen
 * @version 1.0
 */
class SimplePlayer implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("native-lib");
    }

    private String dataSource;
    private SurfaceHolder holder;
    private OnPrepareListener listener;

    public native String getVersionNative();

    private native void prepareFFmpegNative(String dataSource);

    private native void nativeStartPlay();

    private native void nativeSetPlayView(Surface surface);

    public void prepare(String dataSource) {
        this.dataSource = dataSource;
        prepareFFmpegNative(dataSource);
    }

    public void startPlay() {
        nativeStartPlay();
    }

    public void stop() {

    }

    public void release() {
        holder.removeCallback(this);
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        holder = surfaceView.getHolder();
        holder.addCallback(this);
    }

    public void onError(int errorCode) {
        Log.e("SimplePlayer", "Java接到回调:" + errorCode);
        if(listener != null){
            listener.onError(errorCode);
        }

    }

    public void onPrepare() {
        Log.e("SimplePlayer", "onPrepareSuccess");
        if(listener != null){
            listener.onPrepare();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nativeSetPlayView(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void setOnPrepareListener(OnPrepareListener listener) {
        this.listener = listener;
    }

    public interface OnPrepareListener {
        void onPrepare();
        void onError(int errorCode);
    }
}
