//
// Created by Doom119 on 16/2/26.
//

#include "FFmpegPlayer.h"
#include "utils/Mutex.h"
#include "utils/List.h"

using namespace FFS;

void avlog_callback(void *x, int level, const char *fmt, va_list ap)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ap);
}

int FFmpegPlayer::init(IRenderer *renderer, IAudio* audio)
{
    LOGD("FFmpegPlayer init");
    if(m_bIsInited)
    {
        LOGD("FFmpegPlayer has already initiated");
        return 0;
    }

    if(NULL == renderer)
    {
        LOGW("FFmpegPlayer init, renderer can not be NULL");
        return PLAYER_ERROR_INIT;
    }

    if(NULL == audio)
    {
        LOGW("FFmpegPlayer init, audio can not be NULL");
        return PLAYER_ERROR_INIT;
    }

    m_pRenderer = renderer;
    m_pRenderer->init();

    m_pAudio = audio;
    m_pAudio->init();

    av_register_all();
    av_log_set_callback(avlog_callback);

    m_bIsInited = true;

    return 0;
}

IRenderer* FFmpegPlayer::getRenderer()
{
    LOGD("FFmpegPlayer getRenderer");
    return m_pRenderer;
}

int FFmpegPlayer::open(const char *filename)
{
    av_strlcpy(m_aFileName, filename, sizeof(m_aFileName));
    LOGD("FFmpegPlayer open file=%s", m_aFileName);

    int ret = avformat_open_input(&m_pFormatCtx, m_aFileName, NULL, NULL);
    if (ret)
    {
//        LOGE("avformat_open_input error:%d, %s", ret, av_err2str(ret));
        LOGW("avformat_open_input error, %d", ret);
        return PLAYER_ERROR_OPEN;
    }

    ret = avformat_find_stream_info(m_pFormatCtx, NULL);
    if (ret)
    {
//        LOGD("avformat_find_stream_info error, %s", av_err2str(ret));
        LOGW("avformat_find_stream_info error, %d", ret);
        return PLAYER_ERROR_OPEN;
    }

    int i = 0;
    for(; i < m_pFormatCtx->nb_streams; ++i)
    {
        if(AVMEDIA_TYPE_VIDEO == m_pFormatCtx->streams[i]->codec->codec_type)
        {
            m_nVideoStream = i;

            AVCodecContext *pCodecCtx = m_pFormatCtx->streams[i]->codec;
            m_pVideoCodec = avcodec_find_decoder(pCodecCtx->codec_id);
            //note that we must not use AVCodecContext from the
            //video stream directly according to dranger's tutorial
            m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
            ret = avcodec_copy_context(m_pVideoCodecCtx, pCodecCtx);
            if (ret)
            {
        //        LOGD("AVCodecContext copy is NULL, %s", av_err2str(ret));
                LOGW("AVCodecContext video copy failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }

            ret = avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL);
            if (ret)
            {
        //        LOGD("avcodec_open2 error, %s", av_err2str(ret));
                LOGW("avcodec_open2 video failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }
        }
        else if(AVMEDIA_TYPE_AUDIO == m_pFormatCtx->streams[i]->codec->codec_type)
        {
            m_nAudioStream = i;

            AVCodecContext *pCodecCtx = m_pFormatCtx->streams[i]->codec;
            m_pAudioCodec = avcodec_find_decoder(pCodecCtx->codec_id);
            //note that we must not use AVCodecContext from the
            //video stream directly according to dranger's tutorial
            m_pAudioCodecCtx = avcodec_alloc_context3(m_pAudioCodec);
            ret = avcodec_copy_context(m_pAudioCodecCtx, pCodecCtx);
            if (ret)
            {
                //        LOGD("AVCodecContext copy is NULL, %s", av_err2str(ret));
                LOGW("AVCodecContext audio copy failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }

            ret = avcodec_open2(m_pAudioCodecCtx, m_pAudioCodec, NULL);
            if (ret)
            {
                //        LOGD("avcodec_open2 error, %s", av_err2str(ret));
                LOGW("avcodec_open2 audio failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }
        }
    }

    LOGD("video stream index=%d, audio stream index=%d", m_nVideoStream, m_nAudioStream);

    dump();

    return 0;
}

int FFmpegPlayer::play()
{
    LOGD("FFmpegPlayer play");

    AVFrame *pFrameData = NULL;
    AVFrame *pFrameYUV = NULL;
    AVFrame *pFrameAudio = NULL;
    AVPacket packet;
    int got;
    void *buffer = NULL;
    struct swsContext *imgSwsCtx = NULL;

    imgSwsCtx = (struct swsContext *)sws_getContext(
                           m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                           m_pVideoCodecCtx->pix_fmt,
                           m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                           AV_PIX_FMT_YUV420P,
                           SWS_BICUBIC, NULL, NULL, NULL);
    if (!imgSwsCtx)
    {
        LOGW("sws_getCachedContext failed");
        return PLAYER_ERROR_PLAY;
    }
    pFrameData = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    pFrameAudio = av_frame_alloc();

    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pVideoCodecCtx->width,
                                      m_pVideoCodecCtx->height);
    buffer = av_malloc(numBytes);
    avpicture_fill((AVPicture *) pFrameYUV, (const uint8_t *)buffer, AV_PIX_FMT_YUV420P,
                   m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);

    m_pRenderer->createSurface(m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
    while (av_read_frame(m_pFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == m_nVideoStream)
        {
            avcodec_decode_video2(m_pVideoCodecCtx, pFrameData, &got, &packet);
            if (got)
            {
                sws_scale((SwsContext *) imgSwsCtx, (const uint8_t *const *) pFrameData->data,
                          pFrameData->linesize, 0, m_pVideoCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                m_pRenderer->render(pFrameYUV->data[0], pFrameYUV->linesize[0],
                                    pFrameYUV->data[1], pFrameYUV->linesize[1],
                                    pFrameYUV->data[2], pFrameYUV->linesize[2]);
            }
            av_free_packet(&packet);
        }
        else if(packet.stream_index == m_nAudioStream)
        {
            int len = avcodec_decode_audio4(m_pAudioCodecCtx, pFrameAudio, &got, &packet);
            if (len < 0)
            {
                LOGW("avcodec_decode_audio4 failed");
                continue;
            }
            if (got)
            {
                int plane_size;
                int planar = av_sample_fmt_is_planar(m_pAudioCodecCtx->sample_fmt);
                int data_size = av_samples_get_buffer_size(&plane_size, m_pAudioCodecCtx->channels,
                                           pFrameAudio->nb_samples, m_pAudioCodecCtx->sample_fmt, 1);
                LOGD("planar=%d, plane_size=%d, data_size=%d", planar, plane_size, data_size);

                m_pAudio->play(pFrameAudio->data[0], pFrameAudio->linesize[0]);
            }
            av_free_packet(&packet);
        }
    }
    m_pRenderer->destroySurface();

    sws_freeContext((SwsContext *) imgSwsCtx);
    av_free(buffer);
    av_frame_free(&pFrameAudio);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrameData);
    return 0;
}

int FFmpegPlayer::pause()
{
    return 0;
}

int FFmpegPlayer::resume()
{
    return 0;
}

int FFmpegPlayer::stop()
{
    return 0;
}

int FFmpegPlayer::close()
{
    avcodec_close(m_pVideoCodecCtx);
    avformat_close_input(&m_pFormatCtx);
    return 0;
}

void FFmpegPlayer::dump()
{
    unsigned int nb_streams = m_pFormatCtx->nb_streams;
    int bit_rate = m_pFormatCtx->bit_rate;
    int duration = m_pFormatCtx->duration;
    int packet_size = m_pFormatCtx->packet_size;
    void *streams_addr = m_pFormatCtx->streams;
    LOGD("********* begin dump AVFormatContext ***********");
    LOGD("bit_rate=%d, nb_streams=%d, duration=%d, packet_size=%d", bit_rate, nb_streams, duration,
         packet_size);
    LOGD("streams_addr=%u", streams_addr);
    LOGD("********* end dump AVFormatContext *************");

    int i = 0;
    for (; i < m_pFormatCtx->nb_streams; ++i) {
        LOGD(" ");
        LOGD("******** begin dump AVCodecContext, stream=%d ********", i);
        AVCodecContext *pCodeCtx = m_pFormatCtx->streams[i]->codec;
        int codec_type = pCodeCtx->codec_type;
        int coded_width = pCodeCtx->coded_width;
        int coded_height = pCodeCtx->coded_height;
        int width = pCodeCtx->width;
        int height = pCodeCtx->height;
        int codec_id = pCodeCtx->codec_id;
        int bit_rate = pCodeCtx->bit_rate;
        int gop_size = pCodeCtx->gop_size;
        int sampleRate = pCodeCtx->sample_rate;
        int channels = pCodeCtx->channels;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width,
             coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("sample rate=%d, channels=%d", sampleRate, channels);
        LOGD("width=%d, height=%d", width, height);
        LOGD("******** end dump AVCodecContext ********");
    }
}