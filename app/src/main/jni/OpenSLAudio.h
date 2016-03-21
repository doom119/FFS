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
#include "utils/Condition.h"
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
            m_bIsFirst = true;
            m_nPlayedBytes = 0;
            m_nSampleRate = 48000;
            m_nChannels = 1;
        }

        virtual ~OpenSLAudio(){}

    public:
        int init(uint32_t sample_rate, uint32_t channels);
        int play(uint8_t* data, uint32_t size);
        double getClock();
        List<AudioData*>& getAudioDataList() { return m_audioDataList; }
        Mutex& getMutex() { return m_mutex; }
        Condition& getCondition() { return m_condition; }

        static void playerCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context);

    private:
        uint64_t getPlayedBytes() { return m_nPlayedBytes; }
        void setPlayedBytes(uint64_t bytes) { m_nPlayedBytes = bytes; }

    private:
        SLObjectItf m_engineObject;
        SLObjectItf m_outputObject;
        SLObjectItf m_playerObject;

        SLEngineItf m_engineInterface;
        SLPlayItf m_playInterface;
        SLEffectSendItf m_effectSendInterface;
        SLVolumeItf m_volumeInterface;

        SLAndroidSimpleBufferQueueItf m_bufferQueue;

        Mutex m_mutex;
        Condition m_condition;

        List<AudioData*> m_audioDataList;
        bool m_bIsFirst;

        uint64_t m_nPlayedBytes;
        uint32_t m_nSampleRate;
        uint32_t m_nChannels;
    };

};

#endif //FFS_OPENSLAUDIO_H
