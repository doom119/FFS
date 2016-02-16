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

    public static native void init();
}
