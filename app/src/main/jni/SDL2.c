//
// Created by Doom119 on 16/2/25.
//
#include "FFS.h"
#include "com_doom119_ffs_SDL2.h"
#include "SDL2/SDL.h"

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_SDL2_nativeInit(JNIEnv *env, jclass cls)
{
    LOGD("SDL2 init");

    SDL_Android_Init(env, cls);
    SDL_SetMainReady();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        LOGW("SDL_Init error, %s", SDL_GetError());
    }
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_nativeLowMemory(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_nativeLowMemory(env, cls);
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_nativeQuit(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_nativeQuit(env, cls);
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_nativePause(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_nativePause(env, cls);
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_nativeResume(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_nativeResume(env, cls);
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_onNativeDropFile(JNIEnv *env, jclass cls, jstring filename)
{
    Java_org_libsdl_app_SDLActivity_onNativeDropFile(env, cls, filename);
}

JNIEXPORT void JNICALL 
Java_com_doom119_ffs_SDL2_onNativeResize(JNIEnv *env, jclass cls, jint x, jint y, jint format, jfloat rate)
{
    Java_org_libsdl_app_SDLActivity_onNativeResize(env, cls, x, y, format, rate);
}

JNIEXPORT jint JNICALL 
Java_com_doom119_ffs_SDL2_onNativePadDown(JNIEnv *env, jclass cls, jint device_id, jint keycode)
{
    Java_org_libsdl_app_SDLActivity_onNativePadDown(env, cls, device_id, keycode);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_SDL2_onNativePadUp(JNIEnv *env, jclass cls, jint device_id, jint keycode)
{
    Java_org_libsdl_app_SDLActivity_onNativePadUp(env, cls, device_id, keycode);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeJoy(JNIEnv *env, jclass cls, jint device_id, jint axis, jfloat value)
{
    Java_org_libsdl_app_SDLActivity_onNativeJoy(env, cls, device_id, axis, value);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeHat(JNIEnv *env, jclass cls, jint device_id, jint hat_id, jint x, jint y)
{
    Java_org_libsdl_app_SDLActivity_onNativeHat(env, cls, device_id, hat_id, x, y);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeKeyDown(JNIEnv *env, jclass cls, jint keycode)
{
    Java_org_libsdl_app_SDLActivity_onNativeKeyDown(env, cls, keycode);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeKeyUp(JNIEnv *env, jclass cls, jint keycode)
{
    Java_org_libsdl_app_SDLActivity_onNativeKeyUp(env, cls, keycode);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeKeyboardFocusLost(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_onNativeKeyboardFocusLost(env, cls);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeMouse(JNIEnv *env, jclass cls, jint button, jint action, jfloat x, jfloat y)
{
    Java_org_libsdl_app_SDLActivity_onNativeMouse(env, cls, button, action, x, y);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeTouch(JNIEnv *env, jclass cls,
              jint touchDevId, jint pointerFingerId, jint action, jfloat x, jfloat y, jfloat p)
{
    Java_org_libsdl_app_SDLActivity_onNativeTouch(env, cls,
                touchDevId, pointerFingerId, action, x, y, p);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeAccel(JNIEnv *env, jclass cls, jfloat x, jfloat y, jfloat z)
{
    Java_org_libsdl_app_SDLActivity_onNativeAccel(env, cls, x, y, z);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeSurfaceChanged(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_onNativeSurfaceChanged(env, cls);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_SDL2_onNativeSurfaceDestroyed(JNIEnv *env, jclass cls)
{
    Java_org_libsdl_app_SDLActivity_onNativeSurfaceDestroyed(env, cls);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_SDL2_nativeAddJoystick(JNIEnv *env, jclass cls,
                jint device_id, jstring name, jint is_accelerometer,
                jint nbuttons, jint naxes, jint nhats, jint nballs)
{
    Java_org_libsdl_app_SDLActivity_nativeAddJoystick(env, cls,
                device_id, name, is_accelerometer,
                nbuttons, naxes, nhats, nballs);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_SDL2_nativeRemoveJoystick(JNIEnv *env, jclass cls, jint device_id)
{
    Java_org_libsdl_app_SDLActivity_nativeRemoveJoystick(env, cls, device_id);
}

JNIEXPORT jstring JNICALL
Java_com_doom119_ffs_SDL2_nativeGetHint(JNIEnv *env, jclass cls, jstring name)
{
    Java_org_libsdl_app_SDLActivity_nativeGetHint(env, cls, name);
}

