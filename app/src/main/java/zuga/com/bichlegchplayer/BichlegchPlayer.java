package zuga.com.bichlegchplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * @author saqrag
 * @version 1.0
 * @mail saqrag@outlook.com
 * @see null
 * 2020-01-28
 * @since 1.0
 */
public class BichlegchPlayer implements SurfaceHolder.Callback {
    private SurfaceHolder mSurfaceHolder;
    private String mDataSource;
    private OnPreparedListener mOnPreparedListener;

    static {
        System.loadLibrary("BichlegchPlayer");
    }

    void setSurfaceView(SurfaceView surfaceView) {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder = surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    void setDataSource(String source) {
        mDataSource = source;
    }

    void prepare() {
        nativePrepare(mDataSource);
    }

    void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOnPreparedListener = onPreparedListener;
    }

    public void start() {
        nativeStart();
    }

    public void release() {
        nativeRelease();
    }

    interface OnPreparedListener {
        void onPrepared();

        void onError(int errorMode);
    }


    public void onPrepared() {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    public void onErrorActions(int errorMode) {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onError(errorMode);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nativeSetSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    private native void nativeSetSurface(Surface surface);

    private native void nativePrepare(String dataSource);

    private native void nativeStart();

    private native void nativeRelease();
}
