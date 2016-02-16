//
// Created by Doom119 on 16/1/27.
//
#include "com_doom119_ffs_Loader.h"
#include "libavformat/avformat.h"

JNIEXPORT void JNICALL Java_com_doom119_ffs_FFS_init
(JNIEnv *env, jclass clazz)
{
    av_register_all();
}