package com.doom119.ffs;

import android.view.SurfaceView;

/**
 * Created by doom119 on 16/1/27.
 */
public class FFS
{
    static
    {
        System.loadLibrary("FFS");
        System.loadLibrary("SDL2");
//        System.loadLibrary("avcodec-56");
//        System.loadLibrary("avfilter-5");
//        System.loadLibrary("avformat-56");
//        System.loadLibrary("avutil-54");
//        System.loadLibrary("swresample-1");
//        System.loadLibrary("swscale-3");
    }

    public static long nativePlayerPtr;//native player pointer
    private static native int nativeInit(String renderClass);
    private static native int nativeOpen(String filename);
    private static native int nativePlay();
    private static native int nativeClose();

    public static int init(SurfaceView surfaceView)
    {
        SDL2.init(surfaceView);
        return nativeInit("com/doom119/ffs/SDL2");
    }

    public static int open(String filename)
    {
        return nativeOpen(filename);
    }

    public static int play()
    {
        return nativePlay();
    }

    public static int close()
    {
        return nativeClose();
    }
}
