package com.doom119.ffs;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceView;

/**
 * Created by doom119 on 16/2/25.
 */
public class SDL2
{
    public static boolean mSeparateMouseAndTouch;

    public static SurfaceView mSurfaceView;

    public static int init(SurfaceView view)
    {
        mSurfaceView = view;

        return 0;
    }

    public static void close()
    {
        nativeQuit();
        mSurfaceView = null;
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static Surface getNativeSurface() {
        if(null == mSurfaceView)
            throw new NullPointerException("SurfaceView can not be null");
        return mSurfaceView.getHolder().getSurface();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static int audioInit(int sampleRate, boolean is16Bit, boolean isStereo, int desiredFrames) {
        return 0;
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioWriteShortBuffer(short[] buffer) {

    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioWriteByteBuffer(byte[] buffer) {

    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioQuit() {

    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void pollInputDevices() {

    }

    /**
     * This method is called by SDL using JNI.
     * @return an array which may be empty but is never null.
     */
    public static int[] inputGetInputDeviceIds(int sources) {
        return null;
    }


    /**
     * This method is called by SDL using JNI.
     */
    public static boolean sendMessage(int command, int param) {
        return false;
    }


    /**
     * This method is called by SDL using JNI.
     */
    public static boolean showTextInput(int x, int y, int w, int h) {
        return false;
    }


    /**
     * This method is called by SDL using JNI.
     */
    public static Context getContext() {
        return null;
    }


    /**
     * This method is called by SDL using JNI.
     */
    public static boolean setActivityTitle(String title) {
        return false;
    }

    public static native int nativeInit();
    public static native void nativeLowMemory();
    public static native void nativeQuit();
    public static native void nativePause();
    public static native void nativeResume();
    public static native void onNativeDropFile(String filename);
    public static native void onNativeResize(int x, int y, int format, float rate);
    public static native int onNativePadDown(int device_id, int keycode);
    public static native int onNativePadUp(int device_id, int keycode);
    public static native void onNativeJoy(int device_id, int axis,
                                          float value);
    public static native void onNativeHat(int device_id, int hat_id,
                                          int x, int y);
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeKeyboardFocusLost();
    public static native void onNativeMouse(int button, int action, float x, float y);
    public static native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x,
                                            float y, float p);
    public static native void onNativeAccel(float x, float y, float z);
    public static native void onNativeSurfaceChanged();
    public static native void onNativeSurfaceDestroyed();
    public static native int nativeAddJoystick(int device_id, String name,
                                               int is_accelerometer, int nbuttons,
                                               int naxes, int nhats, int nballs);
    public static native int nativeRemoveJoystick(int device_id);
    public static native String nativeGetHint(String name);
}
