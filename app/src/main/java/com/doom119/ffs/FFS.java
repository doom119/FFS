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
//        System.loadLibrary("avcodec-56");
//        System.loadLibrary("avfilter-5");
//        System.loadLibrary("avformat-56");
//        System.loadLibrary("avutil-54");
//        System.loadLibrary("swresample-1");
//        System.loadLibrary("swscale-3");
    }

    public static int init(SurfaceView surfaceView)
    {
        int ret = 0;
        ret = FFMPEG.init();
        SDL2.init(surfaceView);
        return ret;
    }

    public static int open(String videoPath)
    {
        int ret = 0;
        ret = FFMPEG.open(videoPath);
        return ret;
    }

    public static int decode()
    {
        int ret = 0;
        ret = FFMPEG.decode();
        return ret;
    }

    public static void close()
    {
        FFMPEG.close();
        SDL2.close();
    }
}
