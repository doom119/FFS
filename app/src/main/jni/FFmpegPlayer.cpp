//
// Created by Doom119 on 16/2/26.
//

#include <SDL2/SDL_timer.h>
#include "FFmpegPlayer.h"
#include "utils/Mutex.h"

using namespace FFS;
Mutex gPlayerMutex;

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

int FFmpegPlayer::open(const char *filename)
{
    av_strlcpy(m_aFileName, filename, sizeof(m_aFileName));
    LOGD("FFmpegPlayer open file=%s", m_aFileName);

    int ret = avformat_open_input(&m_pFormatCtx, m_aFileName, NULL, NULL);
    if (ret)
    {
        LOGW("avformat_open_input error, %d", ret);
        return PLAYER_ERROR_OPEN;
    }

    ret = avformat_find_stream_info(m_pFormatCtx, NULL);
    if (ret)
    {
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
                LOGW("AVCodecContext video copy failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }

            ret = avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL);
            if (ret)
            {
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
                LOGW("AVCodecContext audio copy failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }

            ret = avcodec_open2(m_pAudioCodecCtx, m_pAudioCodec, NULL);
            if (ret)
            {
                LOGW("avcodec_open2 audio failed, %d", ret);
                return PLAYER_ERROR_OPEN;
            }
        }
    }

    LOGD("video stream index=%d, audio stream index=%d", m_nVideoStream, m_nAudioStream);

    m_pSwsContext = sws_getContext(
                         m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                         m_pVideoCodecCtx->pix_fmt,
                         m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                         AV_PIX_FMT_YUV420P,
                         SWS_BICUBIC, NULL, NULL, NULL);
    m_pSwrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, 48000,
                         m_pAudioCodecCtx->channel_layout, m_pAudioCodecCtx->sample_fmt,
                         m_pAudioCodecCtx->sample_rate, 0, NULL);
    swr_init(m_pSwrContext);

    dump();

    return 0;
}

int FFmpegPlayer::play()
{
    m_bIsDecodeFinished = false;
    m_decodeThread.start(decodeInternal, this);
    m_playThread.start(playInternal, this);
}

void* FFmpegPlayer::decodeInternal(void* args)
{
    LOGD("FFmpegPlayer decodeInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVFormatContext *pFormatCtx = pPlayer->getFormatContext();
    int videoStream = pPlayer->getVideoStreamIndex();
    int audioStream = pPlayer->getAudioStreamIndex();
    List<AVPacket>* videoPacketList = pPlayer->getVideoPacketList();
    List<AVPacket>* audioPacketList = pPlayer->getAudioPacketList();
    AVPacket packet;

    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == videoStream)
        {
            gPlayerMutex.lock();
            videoPacketList->push_back(packet);
            LOGD("decodeInternal, video size=%d", videoPacketList->size());
            gPlayerMutex.unlock();
        }
        else if(packet.stream_index == audioStream)
        {
            gPlayerMutex.lock();
            audioPacketList->push_back(packet);
            LOGD("decodeInternal, audio size=%d", audioPacketList->size());
            gPlayerMutex.unlock();
        }
        else
        {
            av_free_packet(&packet);
        }
    }

//    pPlayer->setDecodeFinished(true);
}

void* FFmpegPlayer::playInternal(void* args)
{
    LOGD("FFmpegPlayer playInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVCodecContext *pVideoCodecCtx = pPlayer->getVideoCodecContext();
    AVCodecContext *pAudioCodecCtx = pPlayer->getAudioCodecContext();
    SwrContext* pSwrContext = pPlayer->getSwrContext();
    SwsContext* pSwsContext = pPlayer->getSwsContext();
    IRenderer* pRenderer = pPlayer->getRenderer();
    IAudio* pAudio = pPlayer->getAudio();
    List<AVPacket>* videoPacketList = pPlayer->getVideoPacketList();
    List<AVPacket>* audioPacketList = pPlayer->getAudioPacketList();

    AVFrame *pFrameData = NULL;
    AVFrame *pFrameYUV = NULL;
    AVFrame *pFrameAudio = NULL;
    void *buffer = NULL;
    int got = 0;
    uint8_t audio_buf[AVCODEC_MAX_AUDIO_FRAME_SIZE * 2];

    pFrameData = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    pFrameAudio = av_frame_alloc();

    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pVideoCodecCtx->width,
                                      pVideoCodecCtx->height);
    buffer = av_malloc(numBytes);
    avpicture_fill((AVPicture *) pFrameYUV, (const uint8_t *)buffer, AV_PIX_FMT_YUV420P,
                   pVideoCodecCtx->width, pVideoCodecCtx->height);
    pRenderer->createSurface(pVideoCodecCtx->width, pVideoCodecCtx->height);
    while(!pPlayer->isDecodeFinished())
    {
        gPlayerMutex.lock();
        if(videoPacketList->size() > 0)
        {
//            LOGD("playInternal, video size=%d", videoPacketList->size());
            List<AVPacket>::iterator it = videoPacketList->begin();
            videoPacketList->erase(it);
            AVPacket packet = *it;

            int len = avcodec_decode_video2(pVideoCodecCtx, pFrameData, &got, &packet);
            if(len < 0)
            {
                LOGW("FFmpegPlayer, avcodec_decode_video2 error");
                continue;
            }
            if (got)
            {
                sws_scale(pSwsContext, (const uint8_t *const *) pFrameData->data,
                          pFrameData->linesize, 0, pVideoCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                pRenderer->render(pFrameYUV->data[0], pFrameYUV->linesize[0],
                                    pFrameYUV->data[1], pFrameYUV->linesize[1],
                                    pFrameYUV->data[2], pFrameYUV->linesize[2]);
            }

            av_free_packet(&packet);
        }

        if(audioPacketList->size() > 0)
        {
            LOGD("playInternal, audio size=%d", audioPacketList->size());
            List<AVPacket>::iterator it = audioPacketList->begin();
            audioPacketList->erase(it);

            AVPacket packet = *it;

            int audio_size = decodeAudioFrame(pAudioCodecCtx, pSwrContext, packet, audio_buf,
                                              sizeof(audio_buf));
            pAudio->play(audio_buf, audio_size);

            av_free_packet(&packet);
        }
        gPlayerMutex.unlock();
        SDL_Delay(50);
    }
    pRenderer->destroySurface();

    av_free(buffer);
    av_frame_free(&pFrameAudio);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrameData);
    LOGD("playInternal end");
}

int FFmpegPlayer::decodeAudioFrame(AVCodecContext* pCodecCtx, SwrContext* pSwrContext,
                                   AVPacket &pkt, uint8_t *audio_buf, int buf_size)
{
    LOGD("FFmpegPlayer decodeAudioFrame, buf size=%d", buf_size);
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;
    static uint32_t data_size = 0;
    static uint32_t len1 = 0;

    for (; ;)
    {
        while (audio_pkt_size > 0)
        {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(pCodecCtx, &frame, &got_frame, &pkt);
            if (len1 < 0)
            {
                /* if error, skip frame */
                LOGW("decodeAudioFrame, len1=%d", len1);
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame)
            {
                data_size = audioResampling(pCodecCtx, pSwrContext, &frame, AV_SAMPLE_FMT_S16, frame.channels, frame.sample_rate, audio_buf);
//                LOGD("FFmpegPlayer decodeAudioFrame, data_size=%d", data_size);
            }
            if (data_size <= 0)
            {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }

        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
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
    sws_freeContext(m_pSwsContext);
    swr_free(&m_pSwrContext);
    avcodec_close(m_pVideoCodecCtx);
    avformat_close_input(&m_pFormatCtx);
    m_bIsInited = false;
    m_bIsDecodeFinished = true;
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
        int sample_rate = pCodeCtx->sample_rate;
        int channels = pCodeCtx->channels;
        int channel_layout = pCodeCtx->channel_layout;
        int samples_fmt = pCodeCtx->sample_fmt;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width,
             coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("sample rate=%d, channels=%d, channel_layout=%d, samples_fmt=%d", sample_rate, channels, channel_layout, samples_fmt);
        LOGD("width=%d, height=%d", width, height);
        LOGD("******** end dump AVCodecContext ********");
    }
}

int FFmpegPlayer::audioResampling(AVCodecContext* pCodecCtx, SwrContext* pSwrContext, AVFrame * pAudioDecodeFrame,
                           int out_sample_fmt,
                           int out_channels,
                           int out_sample_rate,
                           uint8_t* out_buf)
{
    int ret = 0;
    int64_t src_ch_layout = pCodecCtx->channel_layout;
    int64_t dst_ch_layout = AV_CH_LAYOUT_MONO;
    int dst_nb_channels = 0;
    int dst_linesize = 0;
    int src_nb_samples = 0;
    int dst_nb_samples = 0;
    int max_dst_nb_samples = 0;
    uint8_t **dst_data = NULL;
    int resampled_data_size = 0;

    src_ch_layout = (pCodecCtx->channels ==
                     av_get_channel_layout_nb_channels(pCodecCtx->channel_layout)) ?
                    pCodecCtx->channel_layout :
                    av_get_default_channel_layout(pCodecCtx->channels);

    if (out_channels == 1)
    {
        dst_ch_layout = AV_CH_LAYOUT_MONO;
        //printf("dst_ch_layout: AV_CH_LAYOUT_MONO\n");
    }
    else if (out_channels == 2)
    {
        dst_ch_layout = AV_CH_LAYOUT_STEREO;
        //printf("dst_ch_layout: AV_CH_LAYOUT_STEREO\n");
    }
    else
    {
        dst_ch_layout = AV_CH_LAYOUT_SURROUND;
        //printf("dst_ch_layout: AV_CH_LAYOUT_SURROUND\n");
    }

    if (src_ch_layout <= 0)
    {
        LOGW("src_ch_layout error \n");
        return -1;
    }

    src_nb_samples = pAudioDecodeFrame->nb_samples;
    if (src_nb_samples <= 0)
    {
        LOGW("src_nb_samples error \n");
        return -1;
    }

//    av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
//    av_opt_set_int(swr_ctx, "in_sample_rate", m_pAudioCodecCtx->sample_rate, 0);
//    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", m_pAudioCodecCtx->sample_fmt, 0);
//
//    av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
//    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
//    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", (AVSampleFormat)out_sample_fmt, 0);
//
//    if ((ret = swr_init(swr_ctx)) < 0) {
//        LOGW("Failed to initialize the resampling context\n");
//        return -1;
//    }

    max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples,
                                                         out_sample_rate, pCodecCtx->sample_rate, AV_ROUND_UP);
    if (max_dst_nb_samples <= 0)
    {
        LOGW("av_rescale_rnd error \n");
        return -1;
    }

    dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
                                             dst_nb_samples, (AVSampleFormat)out_sample_fmt, 0);
    if (ret < 0)
    {
        LOGW("av_samples_alloc_array_and_samples error \n");
        return -1;
    }


    dst_nb_samples = av_rescale_rnd(swr_get_delay(pSwrContext, pCodecCtx->sample_rate) +
                                    src_nb_samples, out_sample_rate, pCodecCtx->sample_rate, AV_ROUND_UP);
    if (dst_nb_samples <= 0)
    {
        LOGW("av_rescale_rnd error \n");
        return -1;
    }
    if (dst_nb_samples > max_dst_nb_samples)
    {
        av_free(dst_data[0]);
        ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
                               dst_nb_samples, (AVSampleFormat)out_sample_fmt, 1);
        max_dst_nb_samples = dst_nb_samples;
    }

    if (pSwrContext)
    {
        ret = swr_convert(pSwrContext, dst_data, dst_nb_samples,
                          (const uint8_t **)pAudioDecodeFrame->data, pAudioDecodeFrame->nb_samples);
        if (ret < 0)
        {
            LOGW("swr_convert error \n");
            return -1;
        }

        resampled_data_size = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                                         ret, (AVSampleFormat)out_sample_fmt, 1);
        if (resampled_data_size < 0)
        {
            LOGW("av_samples_get_buffer_size error \n");
            return -1;
        }
    }
    else
    {
        LOGW("swr_ctx null error \n");
        return -1;
    }

    memcpy(out_buf, dst_data[0], resampled_data_size);

    if (dst_data)
    {
        av_freep(&dst_data[0]);
    }
    av_freep(&dst_data);
    dst_data = NULL;

    return resampled_data_size;
}
