//
// Created by Doom119 on 16/2/26.
//

#include <SDL2/SDL_timer.h>
#include "FFmpegPlayer.h"
#include "utils/Mutex.h"
#include <sys/time.h>

using namespace FFS;
Mutex gPlayVideoMutex;
Mutex gPlayAudioMutex;

void avlog_callback(void *x, int level, const char *fmt, va_list ap)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ap);
}

int FFmpegPlayer::init(IRenderer *renderer, IAudio* audio)
{
    LOGD("FFmpegPlayer init, relative_time=%llu, time=%llu", av_gettime_relative(), av_gettime());
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
    m_pAudio->init(48000, 1);

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
    m_playVideoThread.start(playVideoInternal, this);
    m_playAudioThread.start(playAudioInternal, this);
    m_decodeThread.start(decodeInternal, this);
    m_displayThread.start(displayInternal, this);
}

void* FFmpegPlayer::decodeInternal(void* args)
{
    LOGD("FFmpegPlayer decodeInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVFormatContext *pFormatCtx = pPlayer->getFormatContext();
    int videoStream = pPlayer->getVideoStreamIndex();
    int audioStream = pPlayer->getAudioStreamIndex();
    List<AVPacket*>* videoPacketList = pPlayer->getVideoPacketList();
    List<AVPacket*>* audioPacketList = pPlayer->getAudioPacketList();
    Mutex& videoMutex = pPlayer->getVideoMutex();
    Condition& videoCnd = pPlayer->getVideoCondition();
    Mutex& audioMutex = pPlayer->getAudioMutex();
    Condition& audioCnd = pPlayer->getAudioCondition();
    AVPacket packet;

    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == videoStream)
        {
            AVPacket *p = new AVPacket();
            av_dup_packet(&packet);
            av_copy_packet(p, &packet);
            videoMutex.lock();
            videoPacketList->push_back(p);
            LOGD("FFmpegPlayer decodeInternal, video size=%d", videoPacketList->size());
            videoCnd.signal();
            videoMutex.unlock();
        }
        else if(packet.stream_index == audioStream)
        {
            AVPacket *p = new AVPacket();
            av_dup_packet(&packet);
            av_copy_packet(p, &packet);
            audioMutex.lock();
            audioPacketList->push_back(p);
            LOGD("FFmpegPlayer decodeInternal, audio size=%d", audioPacketList->size());
            audioCnd.signal();
            audioMutex.unlock();
        }
        else
        {
            av_free_packet(&packet);
        }
    }

//    pPlayer->setDecodeFinished(true);
}

void* FFmpegPlayer::displayInternal(void *args)
{
    LOGD("FFmpegPlayer displayInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVStream* pVideoStream = pPlayer->getVideoStream();
    AVCodecContext *pVideoCodecCtx = pPlayer->getVideoCodecContext();
    SwsContext* pSwsContext = pPlayer->getSwsContext();
    IRenderer* pRenderer = pPlayer->getRenderer();
    IAudio* pAudio = pPlayer->getAudio();
    List<AVFrame*>* displayFrameList = pPlayer->getDisplayFrameList();
    Mutex& displayMutex = pPlayer->getDisplayMutex();
    Condition& displayCnd = pPlayer->getDisplayCondition();

    AVFrame *pFrameYUV = NULL;
    void *buffer = NULL;

    pFrameYUV = av_frame_alloc();

    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pVideoCodecCtx->width,
                                      pVideoCodecCtx->height);
    buffer = av_malloc(numBytes);
    avpicture_fill((AVPicture *) pFrameYUV, (const uint8_t *)buffer, AV_PIX_FMT_YUV420P,
                   pVideoCodecCtx->width, pVideoCodecCtx->height);

    pRenderer->createSurface(pVideoCodecCtx->width, pVideoCodecCtx->height);
    while(!pPlayer->isDecodeFinished())
    {
        AVFrame* pFrame;
        displayMutex.lock();
        if(displayFrameList->size() == 0)
        {
            LOGD("FFmpegPlayer displayInternal wait");
            displayCnd.wait(displayMutex);
        }
        List<AVFrame*>::iterator it = displayFrameList->begin();
        displayFrameList->erase(it);
        pFrame = *it;
        displayMutex.unlock();
        if(NULL == pFrame)
        {
            LOGD("FFmpegPlayer displayInternal, get display frame error");
            continue;
        }

        double lastDelay = pPlayer->getFrameLastDelay();
        double lastPts = pPlayer->getFrameLastPts();
        double frameTimer = pPlayer->getFrameTimer();
        double pts = av_frame_get_best_effort_timestamp(pFrame);
        pts *= av_q2d(pVideoStream->time_base);
//        double frame_delay = av_q2d(pVideoStream->codec->time_base);
        double delay = pts - lastPts;
        if(delay <= 0 || delay >= 1.0)
        {
            delay = lastDelay;
        }
        pPlayer->setFrameLastDelay(delay);
        pPlayer->setFrameLastPts(pts);
        double diff = pts - pAudio->getClock();
        double throld = (delay > 0.01) ? delay : 0.01;
        if(diff <= -throld)
        {
            delay += diff;
        }
        else if(diff >= throld)
        {
            delay *= 2;
        }
        frameTimer += delay;
        pPlayer->setFrameTimer(frameTimer);
        double actual_delay = frameTimer - 1.0 * av_gettime() / AV_TIME_BASE;
        if(actual_delay < 0.01)
            actual_delay = 0.01;
        LOGD("FFmpegPlayer displayInternal pts=%f, lastDelay=%f, lastPts=%f, audio clock=%f, delay=%f, diff=%f, actual_delay=%f", pts, lastDelay, lastPts, pAudio->getClock(), delay, diff, actual_delay);
        av_usleep(actual_delay*1000000);

        sws_scale(pSwsContext, (const uint8_t *const *) pFrame->data,
                  pFrame->linesize, 0, pVideoCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);
        pRenderer->render(pFrameYUV->data[0], pFrameYUV->linesize[0],
                          pFrameYUV->data[1], pFrameYUV->linesize[1],
                          pFrameYUV->data[2], pFrameYUV->linesize[2]);
        av_frame_free(&pFrame);
    }
    pRenderer->destroySurface();

    av_frame_free(&pFrameYUV);
}

void* FFmpegPlayer::playVideoInternal(void* args)
{
    LOGD("FFmpegPlayer playVideoInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVCodecContext *pVideoCodecCtx = pPlayer->getVideoCodecContext();
    List<AVPacket*>* videoPacketList = pPlayer->getVideoPacketList();
    List<AVFrame*>* displayFrameList = pPlayer->getDisplayFrameList();
    Mutex& displayMutex = pPlayer->getDisplayMutex();
    Condition& displayCnd = pPlayer->getDisplayCondition();
    Mutex& videoMutex = pPlayer->getVideoMutex();
    Condition& videoCnd = pPlayer->getVideoCondition();
    int videoIndex = pPlayer->getVideoStreamIndex();

    AVFrame *pFrameData = NULL;
    void *buffer = NULL;
    int got = 0;
    int skipped = 0;

    pFrameData = av_frame_alloc();

    while(!pPlayer->isDecodeFinished())
    {
        AVPacket *pVideoPacket = NULL;

        videoMutex.lock();
        if(videoPacketList->size() == 0)
        {
            LOGD("FFmpegPlayer playVideoInternal wait");
            videoCnd.wait(videoMutex);
        }
        List<AVPacket*>::iterator it = videoPacketList->begin();
        videoPacketList->erase(it);
        pVideoPacket = *it;
        videoMutex.unlock();
        if(pVideoPacket == NULL)
        {
            LOGD("FFmpegPlayer playVideoInternal, get video packet error");
            continue;
        }

        if(pVideoPacket->stream_index == videoIndex)
        {
            struct timeval start, end;
            gettimeofday(&start, NULL);
            LOGD("playVideoInternal, video size=%d, pts=%d", videoPacketList->size(), pVideoPacket->pts);

            int len = avcodec_decode_video2(pVideoCodecCtx, pFrameData, &got, pVideoPacket);
            if(len < 0)
            {
                LOGW("FFmpegPlayer playVideoInternal, avcodec_decode_video2 error");
                continue;
            }
            if (got)
            {
                AVFrame* pFrame = av_frame_clone(pFrameData);

                displayMutex.lock();
                displayFrameList->push_back(pFrame);
                displayCnd.signal();
                displayMutex.unlock();
            }
            else
            {
                skipped ++;
            }

            gettimeofday(&end, NULL);
            int cost = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            LOGD("FFmpegPlayer playVideoInternal, video cost=%d, skipped=%d", cost, skipped);
        }

        av_free_packet(pVideoPacket);
    }

    av_free(buffer);
    av_frame_free(&pFrameData);
}

void* FFmpegPlayer::playAudioInternal(void* args)
{
    LOGD("FFmpegPlayer playAudioInternal");
    FFmpegPlayer* pPlayer = (FFmpegPlayer*)args;
    AVCodecContext *pAudioCodecCtx = pPlayer->getAudioCodecContext();
    SwrContext* pSwrContext = pPlayer->getSwrContext();
    IAudio* pAudio = pPlayer->getAudio();
    List<AVPacket*>* audioPacketList = pPlayer->getAudioPacketList();
    Mutex& audioMutex = pPlayer->getAudioMutex();
    Condition& audioCnd = pPlayer->getAudioCondition();
    int audioIndex = pPlayer->getAudioStreamIndex();

    AVFrame *pFrameAudio = NULL;
    pFrameAudio = av_frame_alloc();
    uint8_t audio_buf[AVCODEC_MAX_AUDIO_FRAME_SIZE * 2];

    while(!pPlayer->isDecodeFinished())
    {
        AVPacket *pAudioPacket = NULL;

        audioMutex.lock();
        if(audioPacketList->size() == 0)
        {
            LOGD("FFmpegPlayer playAudioInternal, wait");
            audioCnd.wait(audioMutex);
        }
        List<AVPacket*>::iterator it = audioPacketList->begin();
        audioPacketList->erase(it);
        pAudioPacket = *it;
        audioMutex.unlock();
        if(pAudioPacket == NULL)
        {
            LOGD("FFmpegPlayer playAudioInternal, get audio packet error");
            continue;
        }

        if(pAudioPacket->stream_index == audioIndex)
        {
            struct timeval start, end;
            gettimeofday(&start, NULL);
            LOGD("FFmpegPlayer playAudioInternal, audio size=%d", audioPacketList->size());

            int audio_size = decodeAudioFrame(pAudioCodecCtx, pSwrContext, pAudioPacket, audio_buf,
                                              sizeof(audio_buf));
            pAudio->play(audio_buf, audio_size);

            gettimeofday(&end, NULL);
            int cost = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            LOGD("FFmpegPlayer playAudioInternal, audio cost=%d", cost);
            av_free_packet(pAudioPacket);
        }
//        SDL_Delay(20);
    }

    av_frame_free(&pFrameAudio);
}

int FFmpegPlayer::decodeAudioFrame(AVCodecContext* pCodecCtx, SwrContext* pSwrContext,
                                   AVPacket *pkt, uint8_t *audio_buf, int buf_size)
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
            len1 = avcodec_decode_audio4(pCodecCtx, &frame, &got_frame, pkt);
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
                uint32_t pts = av_frame_get_best_effort_timestamp(&frame);
                LOGD("FFmpegPlayer playAudioInternal pts=%llu", pts);
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

        audio_pkt_data = pkt->data;
        audio_pkt_size = pkt->size;
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
        int codec_den = pCodeCtx->time_base.den;
        int codec_num = pCodeCtx->time_base.num;
        int stream_den = m_pFormatCtx->streams[i]->time_base.den;
        int stream_num = m_pFormatCtx->streams[i]->time_base.num;
        int max_b_frames = pCodeCtx->max_b_frames;
        int den2 = m_pFormatCtx->streams[i]->r_frame_rate.den;
        int num2 = m_pFormatCtx->streams[i]->r_frame_rate.num;
        LOGD("codec_type=%d, coded_width=%d, coded_height=%d", codec_type, coded_width,
             coded_height);
        LOGD("codec_id=%d, bit_rate=%d, gop_size=%d", codec_id, bit_rate, gop_size);
        LOGD("sample rate=%d, channels=%d, channel_layout=%d, samples_fmt=%d", sample_rate, channels, channel_layout, samples_fmt);
        LOGD("width=%d, height=%d", width, height);
        LOGD("AVCodecContext, time_base.den=%d, time_base.num=%d, max_b_frames=%d", codec_den, codec_num, max_b_frames);
        LOGD("AVStream, time_base.den=%d, time_base.num=%d", stream_den, stream_num);
        LOGD("r_frame_rate.den=%d, r_frame_rate.num=%d", den2, num2);
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
