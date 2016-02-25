package com.doom119.ffs;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

/**
 * Created by doom119 on 16/2/25.
 */
public class SDLSurfaceView extends SurfaceView implements SurfaceHolder.Callback
{
    public static final String TAG = SDLSurfaceView.class.getSimpleName();

    protected static Display mDisplay;

    public SDLSurfaceView(Context context, AttributeSet attrs)
    {
        super(context, attrs);

        getHolder().addCallback(this);
        mDisplay = ((WindowManager)context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
    {
        Log.v(TAG, "surfaceChanged()");

        int sdlFormat = 0x15151002; // SDL_PIXELFORMAT_RGB565 by default
        switch (format) {
            case PixelFormat.A_8:
                Log.v(TAG, "pixel format A_8");
                break;
            case PixelFormat.LA_88:
                Log.v(TAG, "pixel format LA_88");
                break;
            case PixelFormat.L_8:
                Log.v(TAG, "pixel format L_8");
                break;
            case PixelFormat.RGBA_4444:
                Log.v(TAG, "pixel format RGBA_4444");
                sdlFormat = 0x15421002; // SDL_PIXELFORMAT_RGBA4444
                break;
            case PixelFormat.RGBA_5551:
                Log.v(TAG, "pixel format RGBA_5551");
                sdlFormat = 0x15441002; // SDL_PIXELFORMAT_RGBA5551
                break;
            case PixelFormat.RGBA_8888:
                Log.v(TAG, "pixel format RGBA_8888");
                sdlFormat = 0x16462004; // SDL_PIXELFORMAT_RGBA8888
                break;
            case PixelFormat.RGBX_8888:
                Log.v(TAG, "pixel format RGBX_8888");
                sdlFormat = 0x16261804; // SDL_PIXELFORMAT_RGBX8888
                break;
            case PixelFormat.RGB_332:
                Log.v(TAG, "pixel format RGB_332");
                sdlFormat = 0x14110801; // SDL_PIXELFORMAT_RGB332
                break;
            case PixelFormat.RGB_565:
                Log.v(TAG, "pixel format RGB_565");
                sdlFormat = 0x15151002; // SDL_PIXELFORMAT_RGB565
                break;
            case PixelFormat.RGB_888:
                Log.v(TAG, "pixel format RGB_888");
                // Not sure this is right, maybe SDL_PIXELFORMAT_RGB24 instead?
                sdlFormat = 0x16161804; // SDL_PIXELFORMAT_RGB888
                break;
            default:
                Log.v(TAG, "pixel format unknown " + format);
                break;
        }

        Log.v(TAG, "Window size: " + width + "x" + height);
        SDL2.onNativeResize(width, height, sdlFormat, mDisplay.getRefreshRate());
        SDL2.onNativeSurfaceChanged();
        SDL2.nativeResume();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        SDL2.nativePause();
        SDL2.onNativeSurfaceDestroyed();
    }
}
