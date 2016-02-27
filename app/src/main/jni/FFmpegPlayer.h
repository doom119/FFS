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

namespace FFS
{
    class FFmpegPlayer : public IPlayer
    {
    public:
        FFmpegPlayer(const char* renderClass):
                m_sRenderClass(renderClass), m_pFormatCtx(NULL), m_pVideoCodecCtx(NULL)
        {
            memset(m_aFileName, 0, sizeof(m_aFileName));
        }

    public:
        int init(IRenderer *renderer);
        IRenderer* getRenderer();
        int open(const char* filename);
        int play();
        int pause();
        int resume();
        int stop();
        int close();
        void dump();

    public:
        ~FFmpegPlayer()
        {
            if(NULL != m_pRenderer)
                delete m_pRenderer;
        }

    private:
        IRenderer *m_pRenderer;
        const char* m_sRenderClass;
        AVFormatContext *m_pFormatCtx;
        AVCodecContext *m_pVideoCodecCtx;
        AVCodec *m_pVideoCodec;
        int m_nVideoStream;
        int m_nAudioStream;
        char m_aFileName[1024];
    };
};


#endif //FFS_FFMPEGPLAYER_H
