////
//// Created by Doom119 on 16/1/27.
////
//#include <libswscale/swscale.h>
//#include "com_doom119_ffs_FFMPEG.h"
//#include "libavformat/avformat.h"
//#include "FFS.h"
//#include "SDL2/SDL.h"
//
//AVFormatContext* pFormatContext = NULL;
//AVCodecContext* pCodecContext = NULL;
//AVCodec* pCodec = NULL;
//int videoStreamIndex = -1;
//
//void avlog_callback(void *x, int level, const char *fmt, va_list ap) {
//    //LOGD(fmt, ap);
//    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ap);
//}
//
//JNIEXPORT int JNICALL Java_com_doom119_ffs_FFMPEG_init
//        (JNIEnv *env, jclass clazz) {
//    LOGD("init");
//
//    av_register_all();
//    av_log_set_callback(avlog_callback);
//}
//
//JNIEXPORT jint JNICALL
//Java_com_doom119_ffs_FFMPEG_open(JNIEnv *env, jclass clazz, jstring videoPath) {
//    LOGD("open");
//
//    int ret = -1;
//    ret = open(env, videoPath);
//    if (ret < 0)
//        return ret;
//
//    dump();
//}
//
//JNIEXPORT jint JNICALL
//Java_com_doom119_ffs_FFMPEG_decode(JNIEnv *env, jclass clazz) {
//    LOGD("decode");
//    AVFrame *pFrame = NULL;
//    AVFrame *pFrameYUV = NULL;
//    AVPacket packet;
//    void *buffer = NULL;
//    int isFinished;
//    struct swsContext *imgSwsContext = NULL;
//
//    SDL_Window *sdlWindow = NULL;
//    SDL_Texture *sdlTexture = NULL;
//    SDL_Renderer *sdlRenderer = NULL;
//    SDL_Rect sdlRect;
//    SDL_Event sdlEvent;
//
//    imgSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
//                                   pCodecContext->pix_fmt, pCodecContext->width,
//                                   pCodecContext->height,
//                                   AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
//    if (!imgSwsContext) {
//        LOGW("sws_getCachedContext error");
//        return -7;
//    }
//
//    pFrame = av_frame_alloc();
//    if (NULL == pFrame) {
//        LOGD("av_frame_alloc error 1");
//        return -6;
//    }
//
//    pFrameYUV = av_frame_alloc();
//    if (NULL == pFrameYUV) {
//        LOGD("av_frame_alloc error 2");
//        return -6;
//    }
//
//    sdlWindow = SDL_CreateWindow("My Player", SDL_WINDOWPOS_UNDEFINED,
//                                 SDL_WINDOWPOS_UNDEFINED, pCodecContext->width,
//                                 pCodecContext->height, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
//    LOGD("SDL Window=%d", sdlWindow);
//    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
//    LOGD("SDL Renderer=%d", sdlRenderer);
//    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_YV12,
//                                   SDL_TEXTUREACCESS_STREAMING, pCodecContext->width,
//                                   pCodecContext->height);
//    LOGD("SDL Texture=%d", sdlTexture);
//    sdlRect.x = 0;
//    sdlRect.y = 0;
//    sdlRect.w = pCodecContext->width;
//    sdlRect.h = pCodecContext->height;
//
//    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecContext->width,
//                                      pCodecContext->height);
//    buffer = av_malloc(numBytes);
//    avpicture_fill((AVPicture *) pFrameYUV, buffer, AV_PIX_FMT_YUV420P, pCodecContext->width,
//                   pCodecContext->height);
//
//    while (av_read_frame(pFormatContext, &packet) >= 0) {
//        if (packet.stream_index == videoStreamIndex) {
//            avcodec_decode_video2(pCodecContext, pFrame, &isFinished, &packet);
//            if (isFinished) {
//                sws_scale(imgSwsContext, pFrame->data, pFrame->linesize,
//                          0, pCodecContext->height,
//                          pFrameYUV->data, pFrameYUV->linesize);
//                SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
//                                     pFrameYUV->data[0], pFrameYUV->linesize[0],
//                                     pFrameYUV->data[1], pFrameYUV->linesize[1],
//                                     pFrameYUV->data[2], pFrameYUV->linesize[2]);
//                SDL_RenderClear(sdlRenderer);
//                SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
//                SDL_RenderPresent(sdlRenderer);
//                SDL_Delay(16);
//            }
//            av_free_packet(&packet);
//        }
//    }
//    SDL_DestroyTexture(sdlTexture);
//
//    sws_freeContext(imgSwsContext);
//    av_free(buffer);
//    av_frame_free(&pFrameYUV);
//    av_frame_free(&pFrame);
//}
//
//JNIEXPORT void JNICALL
//Java_com_doom119_ffs_FFMPEG_close(JNIEnv *env, jclass clazz) {
//    LOGD("close");
//    avcodec_close(pCodecContext);
//    avformat_close_input(&pFormatContext);
//}
//
//void dump() {
//    unsigned int nb_streams = pFormatContext->nb_streams;
//    int bit_rate = pFormatContext->bit_rate;
//    int duration = pFormatContext->duration;
//    int packet_size = pFormatContext->packet_size;
//    void *streams_addr = pFormatContext->streams;
//    LOGD("********* begin dump AVFormatContext ***********");
//    LOGD("bit_rate=%d, nb_streams=%d, duration=%d, packet_size=%d", bit_rate, nb_streams, duration,
//         packet_size);
//    LOGD("streams_addr=%u", streams_addr);
//    LOGD("********* end dump AVFormatContext *************");
//
//    int i = 0;
//    for (; i < pFormatContext->nb_streams; ++i) {
//        LOGD(" ");
//        LOGD("******** begin dump AVCodecContext, stream=%d ********", i);
//        AVCodecContext *pCodeCtx = pFormatContext->streams[i]->codec;
//        int codec_type = pCodeCtx->codec_type;
//        int coded_width = pCodeCtx->coded_width;
//        int coded_height = pCodeCtx->coded_height;
//        int width = pCodeCtx->width;
//        int height = pCodeCtx->height;
//        int codec_id = pCodeCtx->codec_id;
//        int bit_rate = pCodeCtx->bit_rate;
//        int gop_size = pCodeCtx->gop_size;
//        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width,
//             coded_height);
//        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
//        LOGD("width=%d, height=%d", width, height);
//        LOGD("******** end dump AVCodecContext ********");
//    }
//}
//
//int open(JNIEnv *env, jstring videoPath) {
//    int ret;
//    jboolean isCopy;
//    AVCodecContext *pCodecCtxOrg = NULL;
//
//    const char *path = (*env)->GetStringUTFChars(env, videoPath, &isCopy);
//    LOGD("path=%s", path);
//    ret = avformat_open_input(&pFormatContext, path, NULL, NULL);
//    if (ret) {
//        LOGE("avformat_open_input error:%d, %s", ret, av_err2str(ret));
//        return -1;
//    }
//
//    ret = avformat_find_stream_info(pFormatContext, NULL);
//    if (ret) {
//        LOGD("avformat_find_stream_info error, %s", av_err2str(ret));
//        return -2;
//    }
//
//    int i = 0;
//    for (; i < pFormatContext->nb_streams; ++i) {
//        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoStreamIndex = i;
//        }
//    }
//    pCodecCtxOrg = pFormatContext->streams[videoStreamIndex]->codec;
//    pCodec = avcodec_find_decoder(pCodecCtxOrg->codec_id);
//    if (NULL == pCodecCtxOrg) {
//        LOGD("AVCodecContext original is NULL");
//        return -3;
//    }
//    if (NULL == pCodec) {
//        LOGD("AVCodec is NULL");
//        return -4;
//    }
//
//    //note that we must not use AVCodecContext from the
//    //video stream directly according to dranger's tutorial
//    pCodecContext = avcodec_alloc_context3(pCodec);
//    ret = avcodec_copy_context(pCodecContext, pCodecCtxOrg);
//    if (ret) {
//        LOGD("AVCodecContext copy is NULL, %s", av_err2str(ret));
//        return -3;
//    }
//
//    ret = avcodec_open2(pCodecContext, pCodec, NULL);
//    if (ret) {
//        LOGD("avcodec_open2 error, %s", av_err2str(ret));
//        return -5;
//    }
//    //av_dump_format(pFormatContext, -1, path, 0);
//    (*env)->ReleaseStringUTFChars(env, videoPath, path);
//}