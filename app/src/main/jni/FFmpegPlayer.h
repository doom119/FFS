//
// Created by Doom119 on 16/2/26.
//

#ifndef FFS_FFMPEGPLAYER_H
#define FFS_FFMPEGPLAYER_H

extern "C"
{
#include "libavutil/avstring.h"
#include "libavutil/error.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/error.h"
#include "libavutil/opt.h"
#include <libavutil/time.h>
};

#include "FFS.h"
#include "IPlayer.h"
#include "IAudio.h"
#include "utils/List.h"
#include "utils/Thread.h"
#include "utils/Mutex.h"
#include "utils/Condition.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

namespace FFS
{
    class FFmpegPlayer : public IPlayer
    {
    public:
        FFmpegPlayer(const char* renderClass):
                m_sRenderClass(renderClass), m_pFormatCtx(NULL),
                m_pVideoCodec(NULL), m_pAudioCodec(NULL), m_pAudioCodecCtx(NULL),
                m_pVideoCodecCtx(NULL), m_bIsInited(false), m_pSwrContext(NULL),
                m_pRenderer(NULL), m_pAudio(NULL), m_pSwsContext(NULL)
        {
            memset(m_aFileName, 0, sizeof(m_aFileName));
            m_listVideoPacket = new List<AVPacket*>();
            m_listAudioPacket = new List<AVPacket*>();
            m_listDisplayFrame = new List<AVFrame*>();
            m_frameLastDelay = 0;
            m_frameLastPts = 0;
            m_frameTimer = 1.0 * av_gettime() / AV_TIME_BASE;
        }

    public:
        int init(IRenderer *renderer, IAudio* audio);
        int open(const char* filename);
        int play();
        int pause();
        int resume();
        int stop();
        int close();
        void dump();

    public:
        AVFormatContext* getFormatContext() { return m_pFormatCtx; }
        AVCodecContext* getVideoCodecContext() { return m_pVideoCodecCtx; }
        AVCodecContext* getAudioCodecContext() { return m_pAudioCodecCtx; }
        AVCodec* getVideoCodec() { return m_pVideoCodec; }
        AVCodec* getAudioCodec() { return m_pAudioCodec; }
        SwsContext* getSwsContext() { return m_pSwsContext; }
        SwrContext* getSwrContext() { return m_pSwrContext; }
        IAudio* getAudio() { return m_pAudio; }
        IRenderer* getRenderer() { return m_pRenderer; }
        int getVideoStreamIndex() { return m_nVideoStream; }
        int getAudioStreamIndex() { return m_nAudioStream; }
        AVStream* getVideoStream() { return m_pFormatCtx->streams[m_nVideoStream]; }
        AVStream* getAudioStream() { return m_pFormatCtx->streams[m_nAudioStream]; }
        List<AVPacket*>* getVideoPacketList() { return m_listVideoPacket; }
        List<AVPacket*>* getAudioPacketList() { return m_listAudioPacket; }
        List<AVFrame*>* getDisplayFrameList() { return m_listDisplayFrame; }
        Mutex& getVideoMutex() { return m_videoMutex; }
        Mutex& getAudioMutex() { return m_audioMutex; }
        Mutex& getDisplayMutex() { return m_displayMutex; }
        Condition& getVideoCondition() { return m_videoCnd; }
        Condition& getAudioCondition() { return m_audioCnd; }
        Condition& getDisplayCondition() { return m_displayCnd; }
        bool isDecodeFinished() { return m_bIsDecodeFinished; }
        bool setDecodeFinished(bool b) { m_bIsDecodeFinished = b; }
        double getFrameLastPts() { return m_frameLastPts; }
        double getFrameLastDelay() { return m_frameLastDelay; }
        double getFrameTimer() { return m_frameTimer; }
        void setFrameLastPts(double pts) { m_frameLastPts = pts; }
        void setFrameLastDelay(double delay) { m_frameLastDelay = delay; }
        void setFrameTimer(double t) { m_frameTimer = t; }

    public:
        virtual ~FFmpegPlayer()
        {
            if(NULL != m_pRenderer)
                delete m_pRenderer;
        }

    private:
        static void* decodeInternal(void* args);
        static void* playVideoInternal(void* args);
        static void* playAudioInternal(void* args);
        static void* displayInternal(void* args);
        static int audioResampling(AVCodecContext* pCodecCtx, SwrContext* pSwrContext, AVFrame * pAudioDecodeFrame,
                            int out_sample_fmt,
                            int out_channels,
                            int out_sample_rate,
                            uint8_t* out_buf);
        static int decodeAudioFrame(AVCodecContext* pCodecCtx, SwrContext* pSwrContext, AVPacket *pkt, uint8_t *audio_buf,
                             int buf_size);

    private:
        IRenderer *m_pRenderer;
        IAudio *m_pAudio;

        const char* m_sRenderClass;
        AVFormatContext *m_pFormatCtx;
        AVCodecContext *m_pVideoCodecCtx;
        AVCodec *m_pVideoCodec;
        AVCodecContext *m_pAudioCodecCtx;
        AVCodec *m_pAudioCodec;
        SwsContext *m_pSwsContext;
        SwrContext *m_pSwrContext;
        int m_nVideoStream;
        int m_nAudioStream;
        char m_aFileName[1024];

        double m_frameLastDelay;
        double m_frameLastPts;
        double m_frameTimer;

        Thread m_decodeThread;
        Thread m_playVideoThread;
        Thread m_playAudioThread;
        Thread m_displayThread;

        Mutex m_videoMutex;
        Mutex m_audioMutex;
        Mutex m_displayMutex;
        Condition m_videoCnd;
        Condition m_audioCnd;
        Condition m_displayCnd;

        List<AVPacket*>* m_listVideoPacket;
        List<AVPacket*>* m_listAudioPacket;
        List<AVFrame*>* m_listDisplayFrame;

        bool m_bIsInited;
        bool m_bIsDecodeFinished;
    };
};


#endif //FFS_FFMPEGPLAYER_H
