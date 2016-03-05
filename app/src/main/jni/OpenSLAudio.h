//
// Created by Doom119 on 16/2/29.
//

#include "IAudio.h"
#include <SLES/OpenSLES.h>
#include <stddef.h>
#include <stdint.h>
#include "FFS.h"
#include "utils/List.h"
#include "utils/Mutex.h"
#include <SLES/OpenSLES_Android.h>

#ifndef FFS_OPENSLAUDIO_H
#define FFS_OPENSLAUDIO_H

namespace FFS
{
    typedef struct
    {
        uint8_t *data;
        uint32_t size;
    }AudioData;

    class OpenSLAudio : public IAudio
    {
    public:
        OpenSLAudio() : m_engineObject(NULL), m_outputObject(NULL), m_playerObject(NULL),
                        m_engineInterface(NULL), m_playInterface(NULL),
                        m_effectSendInterface(NULL), m_volumeInterface(NULL)
        {
            m_bIsFirst = false;
        }

        virtual ~OpenSLAudio(){}

    public:
        int init();
        int play(uint8_t* data, uint32_t size);
        static void playerCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context);

    private:
        SLObjectItf m_engineObject;
        SLObjectItf m_outputObject;
        SLObjectItf m_playerObject;

        SLEngineItf m_engineInterface;
        SLPlayItf m_playInterface;
        SLEffectSendItf m_effectSendInterface;
        SLVolumeItf m_volumeInterface;

        SLAndroidSimpleBufferQueueItf m_bufferQueue;

        List<AudioData*> m_audioDataList;
        bool m_bIsFirst;
    };
};

#endif //FFS_OPENSLAUDIO_H
