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
};

#include "FFS.h"
#include "IPlayer.h"
#include "IAudio.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

namespace FFS
{
    class FFmpegPlayer : public IPlayer
    {
    public:
        FFmpegPlayer(const char* renderClass):
                m_sRenderClass(renderClass), m_pFormatCtx(NULL),
                m_pVideoCodec(NULL), m_pAudioCodec(NULL), m_pAudioCodecCtx(NULL),
                m_pVideoCodecCtx(NULL), m_bIsInited(false),
                m_pRenderer(NULL), m_pAudio(NULL)
        {
            memset(m_aFileName, 0, sizeof(m_aFileName));
        }

    public:
        int init(IRenderer *renderer, IAudio* audio);
        IRenderer* getRenderer();
        int open(const char* filename);
        int play();
        int pause();
        int resume();
        int stop();
        int close();
        void dump();

    public:
        virtual ~FFmpegPlayer()
        {
            if(NULL != m_pRenderer)
                delete m_pRenderer;
        }

    private:
        IRenderer *m_pRenderer;
        IAudio *m_pAudio;

        const char* m_sRenderClass;
        AVFormatContext *m_pFormatCtx;
        AVCodecContext *m_pVideoCodecCtx;
        AVCodec *m_pVideoCodec;
        AVCodecContext *m_pAudioCodecCtx;
        AVCodec *m_pAudioCodec;
        int m_nVideoStream;
        int m_nAudioStream;
        char m_aFileName[1024];

        bool m_bIsInited;
    };
};


#endif //FFS_FFMPEGPLAYER_H
