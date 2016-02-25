package com.doom119.ffs;

/**
 * Created by doom119 on 16/2/25.
 */
public class FFMPEG
{
    public static native int init();
    public static native int open(String videoPath);
    public static native int decode();
    public static native void close();
}
