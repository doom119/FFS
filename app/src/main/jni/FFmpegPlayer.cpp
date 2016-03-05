//
// Created by Doom119 on 16/2/26.
//

#include "FFmpegPlayer.h"

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
    uint8_t audio_buf[AVCODEC_MAX_AUDIO_FRAME_SIZE * 2];

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
            int len = avcodec_decode_video2(m_pVideoCodecCtx, pFrameData, &got, &packet);
            if(len < 0)
            {
                LOGW("FFmpegPlayer, avcodec_decode_video2 error");
                continue;
            }
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
            int audio_size = audio_decode_frame(m_pAudioCodecCtx, packet, audio_buf, sizeof(audio_buf));
            m_pAudio->play(audio_buf, audio_size);

//            int len = avcodec_decode_audio4(m_pAudioCodecCtx, pFrameAudio, &got, &packet);
//            if (len < 0)
//            {
//                LOGW("avcodec_decode_audio4 failed");
//                continue;
//            }
//            if (got)
//            {

//                int plane_size;
//                int planar = av_sample_fmt_is_planar(m_pAudioCodecCtx->sample_fmt);
//                int data_size = av_samples_get_buffer_size(&plane_size, m_pAudioCodecCtx->channels,
//                                           pFrameAudio->nb_samples, m_pAudioCodecCtx->sample_fmt, 1);
//                LOGD("planar=%d, plane_size=%d, data_size=%d", planar, plane_size, data_size);
//
//                m_pAudio->play(pFrameAudio->data[0], pFrameAudio->linesize[0]);
//            }
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

int FFmpegPlayer::audio_decode_frame(AVCodecContext *aCodecCtx, AVPacket& pkt,
                                     uint8_t *audio_buf, int buf_size)
{
    LOGD("FFmpegPlayer audio_decode_frame, buf size=%d", buf_size);
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static int out_linesize = 0;
    static AVFrame frame;
    static SwrContext *swrContext = NULL;
    static uint32_t data_size = 0;
    static uint32_t len1 = 0;

    data_size = av_samples_get_buffer_size(&out_linesize,
                                           aCodecCtx->channels,
                                           aCodecCtx->frame_size,
                                           aCodecCtx->sample_fmt,
                                           1);
    if(data_size < 0)
    {
        LOGW("FFmpegPlayer audio_decode_frame, av_samples_get_buffer_size failed");
        return PLAYER_ERROR_PLAY;
    }
    uint8_t out_buffer[data_size];

    swrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                        m_pAudioCodecCtx->channel_layout, m_pAudioCodecCtx->sample_fmt,
                        m_pAudioCodecCtx->sample_rate, 0, NULL);
    if(NULL == swrContext)
    {
        LOGW("FFmpegPlayer audio_decode_frame, SwrContext is NULL");
        return PLAYER_ERROR_PLAY;
    }
    int ret = swr_init(swrContext);
    if(ret < 0)
    {
        LOGW("FFmpegPlayer audio_decode_frame, SwrContext init failed, ret=%d", ret);
        return PLAYER_ERROR_PLAY;
    }

    for (; ;)
    {
        while (audio_pkt_size > 0)
        {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
            if (len1 < 0)
            {
                /* if error, skip frame */
                LOGW("audio_decode_frame, len1=%d", len1);
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame)
            {
//                data_size = av_samples_get_buffer_size(&out_linesize,
//                                                       aCodecCtx->channels,
//                                                       frame.nb_samples,
//                                                       aCodecCtx->sample_fmt,
//                                                       1);
//                memcpy(audio_buf, out_buffer, data_size);

                data_size = AudioResampling(aCodecCtx, &frame, AV_SAMPLE_FMT_S16, frame.channels, frame.sample_rate, audio_buf);
                LOGD("FFmpegPlayer audio_decode_frame, data_size=%d", data_size);
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
        LOGD("FFmpegPlayer audio_decode_frame, pkt.size=%d", pkt.size);
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
        int channel_layout = pCodeCtx->channel_layout;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width,
             coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("sample rate=%d, channels=%d, channel_layout=%d", sampleRate, channels, channel_layout);
        LOGD("width=%d, height=%d", width, height);
        LOGD("******** end dump AVCodecContext ********");
    }
}

int FFmpegPlayer::AudioResampling(AVCodecContext * audio_dec_ctx,
                           AVFrame * pAudioDecodeFrame,
                           int out_sample_fmt,
                           int out_channels,
                           int out_sample_rate,
                           uint8_t* out_buf)
{
    SwrContext * swr_ctx = NULL;
    int data_size = 0;
    int ret = 0;
    int64_t src_ch_layout = audio_dec_ctx->channel_layout;
    int64_t dst_ch_layout = AV_CH_LAYOUT_STEREO;
    int dst_nb_channels = 0;
    int dst_linesize = 0;
    int src_nb_samples = 0;
    int dst_nb_samples = 0;
    int max_dst_nb_samples = 0;
    uint8_t **dst_data = NULL;
    int resampled_data_size = 0;

    swr_ctx = swr_alloc();
    if (!swr_ctx)
    {
        LOGW("swr_alloc error \n");
        return -1;
    }

    src_ch_layout = (audio_dec_ctx->channels ==
                     av_get_channel_layout_nb_channels(audio_dec_ctx->channel_layout)) ?
                    audio_dec_ctx->channel_layout :
                    av_get_default_channel_layout(audio_dec_ctx->channels);

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

    av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", audio_dec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", (AVSampleFormat)out_sample_fmt, 0);

    if ((ret = swr_init(swr_ctx)) < 0) {
        LOGW("Failed to initialize the resampling context\n");
        return -1;
    }

    max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples,
                                                         out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
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


    dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audio_dec_ctx->sample_rate) +
                                    src_nb_samples, out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
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

    if (swr_ctx)
    {
        ret = swr_convert(swr_ctx, dst_data, dst_nb_samples,
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

    if (swr_ctx)
    {
        swr_free(&swr_ctx);
    }
    return resampled_data_size;
}
