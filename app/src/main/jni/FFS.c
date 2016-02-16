//
// Created by Doom119 on 16/1/27.
//
#include "com_doom119_ffs_FFS.h"
#include "libavformat/avformat.h"
#include "FFS.h"

AVFormatContext* pFormatContext = NULL;

JNIEXPORT int JNICALL Java_com_doom119_ffs_FFS_init
(JNIEnv *env, jclass clazz, jstring videoPath)
{
    LOGD("init");
    int ret;

    av_register_all();

    ret = open(env, videoPath);
    if(ret)
        return ret;

    unsigned int nb_streams = pFormatContext->nb_streams;
    int bit_rate = pFormatContext->bit_rate;
    LOGD("bit_rate=%d, nb_streams=%d", bit_rate, nb_streams);
}

int open(JNIEnv *env, jstring videoPath)
{
    int ret;
    jboolean isCopy;

    const char* path = (*env)->GetStringUTFChars(env, videoPath, &isCopy);
    LOGD("path=%s", path);
    ret = avformat_open_input(&pFormatContext, path, NULL, NULL);
    if(ret)
    {
        LOGE("avformat_open_input error:%d, %s", ret, av_err2str(ret));
        return -1;
    }
    (*env)->ReleaseStringUTFChars(env, videoPath, path);

    av_dump_format(pFormatContext, -1, path, 0);
}