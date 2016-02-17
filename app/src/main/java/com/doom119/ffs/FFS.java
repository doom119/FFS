package com.doom119.ffs;

/**
 * Created by doom119 on 16/1/27.
 */
public class FFS
{
    static
    {
        System.loadLibrary("FFS");
    }

    public static native int init();
    public static native int open(String videoPath);
    public static native int decode();
    public static native void close();
}
