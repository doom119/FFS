//
// Created by Doom119 on 16/1/27.
//
#include <libswscale/swscale.h>
#include "com_doom119_ffs_FFS.h"
#include "libavformat/avformat.h"
#include "FFS.h"

AVFormatContext* pFormatContext = NULL;
AVCodecContext* pCodecContext = NULL;
AVCodec* pCodec = NULL;
int videoStreamIndex = -1;

void avlog_callback(void *x, int level, const char *fmt, va_list ap)
{
    LOGD(fmt, ap);
}

JNIEXPORT int JNICALL Java_com_doom119_ffs_FFS_init
(JNIEnv *env, jclass clazz)
{
    LOGD("init");

    av_register_all();
//    av_log_set_callback(avlog_callback);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_open(JNIEnv *env, jclass clazz, jstring videoPath)
{
    LOGD("open");

    int ret = -1;
    ret = open(env, videoPath);
    if(ret<0)
        return ret;

    dump();
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_decode(JNIEnv *env, jclass clazz)
{
    LOGD("decode");
    AVFrame* pFrame = NULL;
    AVFrame* pFrameRGB = NULL;
    AVPacket packet;
    void* buffer = NULL;
    int isFinished;
    struct swsContext* imgSwsContext;

    pFrame = av_frame_alloc();
    if(NULL == pFrame)
    {
        LOGD("av_frame_alloc error 1");
        return -6;
    }

    pFrameRGB = av_frame_alloc();
    if(NULL == pFrameRGB)
    {
        LOGD("av_frame_alloc error 2");
        return -6;
    }

    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height);
    buffer = av_malloc(numBytes);
    avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height);

    //why crash here?
    av_read_frame(pCodecContext, &packet);
//    while(av_read_frame(pCodecContext, &packet) > 0)
//    {
//        if(packet.stream_index == videoStreamIndex)
//        {
//            avcodec_decode_video2(pCodecContext, pFrame, &isFinished, &packet);
//            if(isFinished)
//            {
//                sws_getCachedContext(imgSwsContext, pCodecContext->width, pCodecContext->height,
//                        pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height,
//                        AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
//                if(!imgSwsContext)
//                {
//                    LOGW("sws_getCachedContext error");
//                    return -7;
//                }
//                sws_scale(imgSwsContext, pFrame->data, pFrame->linesize,
//                        0, pCodecContext->height,
//                        pFrameRGB->data, pFrameRGB->linesize);
//            }
//        }
//        av_free_packet(&packet);
//    }

//    sws_freeContext(imgSwsContext);
    av_frame_free(pFrameRGB);
    //av_free(buffer);
    av_frame_free(pFrame);
}

JNIEXPORT void JNICALL
Java_com_doom119_ffs_FFS_close(JNIEnv *env, jclass clazz)
{
    LOGD("close");
    avcodec_close(pCodecContext);
    avformat_close_input(&pFormatContext);
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
        int width = pCodeCtx->width;
        int height = pCodeCtx->height;
        int codec_id = pCodeCtx->codec_id;
        int bit_rate = pCodeCtx->bit_rate;
        int gop_size = pCodeCtx->gop_size;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width, coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("width=%d, height=%d", width, height);
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

    int i = 0;
    for(; i < pFormatContext->nb_streams; ++i)
    {
        if(pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            pCodecContext = pFormatContext->streams[i]->codec;
            pCodec = avcodec_find_decoder(pCodecContext->codec_id);
            videoStreamIndex = i;
        }
    }
    if(NULL == pCodecContext)
    {
        LOGD("AVCodecContext is NULL");
        return -3;
    }
    if(NULL == pCodec)
    {
        LOGD("AVCodec is NULL");
        return -4;
    }

    ret = avcodec_open2(pCodecContext, pCodec, NULL);
    if(ret)
    {
        LOGD("avcodec_open2 error, %s", av_err2str(ret));
        return -5;
    }
//    av_dump_format(pFormatContext, -1, path, 0);
}