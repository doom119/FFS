package com.doom119.ffs;

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

    public static native int init();
    public static native int open(String videoPath);
    public static native int decode();
    public static native void close();
}
