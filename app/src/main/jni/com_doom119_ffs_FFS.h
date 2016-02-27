//
// Created by Doom119 on 16/2/27.
//

#ifndef FFS_COM_DOOM119_FFS_FFS_H_H
#define FFS_COM_DOOM119_FFS_FFS_H_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

JNIEXPORT jint JNICALL
        Java_com_doom119_ffs_FFS_nativeInit(JNIEnv *env, jclass type, jstring renderClass);

JNIEXPORT jint JNICALL
        Java_com_doom119_ffs_FFS_nativeClose(JNIEnv *env, jclass type);

JNIEXPORT jint JNICALL
        Java_com_doom119_ffs_FFS_nativeOpen(JNIEnv *env, jclass type, jstring filename);

JNIEXPORT jint JNICALL
        Java_com_doom119_ffs_FFS_nativePlay(JNIEnv *env, jclass type);

#ifdef __cplusplus
};
#endif

#endif //FFS_COM_DOOM119_FFS_FFS_H_H
