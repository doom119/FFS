//
// Created by Doom119 on 16/1/27.
//
#include "com_doom119_ffs_FFS.h"
#include "libavformat/avformat.h"
#include "FFS.h"

AVFormatContext* pFormatContext = NULL;

void avlog_callback(void *x, int level, const char *fmt, va_list ap)
{
    LOGD(fmt, ap);
}

JNIEXPORT int JNICALL Java_com_doom119_ffs_FFS_init
(JNIEnv *env, jclass clazz, jstring videoPath)
{
    LOGD("init");
    int ret;

    av_register_all();
//    av_log_set_callback(avlog_callback);

    ret = open(env, videoPath);
    if(ret<0)
        return ret;

    dump();

}

void dump()
{
    unsigned int nb_streams = pFormatContext->nb_streams;
    int bit_rate = pFormatContext->bit_rate;
    int duration = pFormatContext->duration;
    int packet_size = pFormatContext->packet_size;
    void* streams_addr = pFormatContext->streams;
    LOGD("********* begin dump AVFormatContext ***********");
    LOGD("bit_rate=%d, nb_streams=%d, duration=%d, packet_size=%d", bit_rate, nb_streams, duration, packet_size);
    LOGD("streams_addr=%u", streams_addr);
    LOGD("********* end dump AVFormatContext *************");

    int i = 0;
    for(; i < pFormatContext->nb_streams; ++i)
    {
        LOGD(" ");
        LOGD("******** begin dump AVCodecContext, stream=%d ********", i);
        AVCodecContext* pCodeCtx = pFormatContext->streams[i]->codec;
        int codec_type = pCodeCtx->codec_type;
        int coded_width = pCodeCtx->coded_width;
        int coded_height = pCodeCtx->coded_height;
        int codec_id = pCodeCtx->codec_id;
        int bit_rate = pCodeCtx->bit_rate;
        int gop_size = pCodeCtx->gop_size;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width, coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("******** end dump AVCodecContext ********");
    }
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

    ret = avformat_find_stream_info(pFormatContext, NULL);
    if(ret)
    {
        LOGD("avformat_find_stream_info error, %s", av_err2str(ret));
        return -2;
    }

//    av_dump_format(pFormatContext, -1, path, 0);
}