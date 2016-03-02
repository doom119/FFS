//
// Created by Doom119 on 16/2/29.
//

#include "OpenSLAudio.h"

using namespace FFS;
int OpenSLAudio::init()
{
    LOGD("OpenSLAudio init");

    SLresult result;
    //create engine object
    result = slCreateEngine(&m_engineObject, 0, NULL, 0, NULL, NULL);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, slCreateEngine, %d", result);
        return AUDIO_ERROR_INIT;
    }

    //realize engine object
    result = (*m_engineObject)->Realize(m_engineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, m_engineObject Realize, %d", result);
        return AUDIO_ERROR_INIT;
    }

    //get engine interface
    result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_ENGINE, &m_engineInterface);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, m_engineObject GetInterface, %d", result);
        return AUDIO_ERROR_INIT;
    }

    //create output mix object
    result = (*m_engineInterface)->CreateOutputMix(m_engineInterface, &m_outputObject, 0, 0, 0);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, m_engineInterface CreateOutputMix, %d", result);
        return AUDIO_ERROR_INIT;
    }

    //realize output mix object
    result = (*m_outputObject)->Realize(m_outputObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, m_outputObject Realize, %d", result);
        return AUDIO_ERROR_INIT;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, //locatorType
            2 //numBuffers
            };
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, //formatType, must be PCM here
            1, //numChannels
            SL_SAMPLINGRATE_8, //samplesPerSec
            SL_PCMSAMPLEFORMAT_FIXED_16, //bitsPerSample
            SL_PCMSAMPLEFORMAT_FIXED_16, //containerSize
            SL_SPEAKER_FRONT_CENTER, //channelMask
            SL_BYTEORDER_LITTLEENDIAN //endianness
            };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {
            SL_DATALOCATOR_OUTPUTMIX, //locatorType
            m_outputObject //outputMix
            };
    SLDataSink audioSink = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*m_engineInterface)->CreateAudioPlayer(
                           m_engineInterface, &m_playerObject,
                           &audioSrc, &audioSink,
                           3, ids, req);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, CreateAudioPlayer object, %d", result);
        return AUDIO_ERROR_INIT;
    }

    result = (*m_playerObject)->Realize(m_playerObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, SLPlayerObject Realize, %d", result);
        return AUDIO_ERROR_INIT;
    }

    result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_PLAY, &m_playInterface);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, SLPlayerObject GetInterface Player, %d", result);
        return AUDIO_ERROR_INIT;
    }

    result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_EFFECTSEND, &m_effectSendInterface);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, SLPlayerObject GetInterface EffectSend, %d", result);
        return AUDIO_ERROR_INIT;
    }

    result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_BUFFERQUEUE, &m_bufferQueue);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, SLPlayerObject GetInterface BufferQueue, %d", result);
        return AUDIO_ERROR_INIT;
    }
    (*m_bufferQueue)->RegisterCallback(m_bufferQueue, OpenSLAudio::playerCallback, NULL);

    result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_VOLUME, &m_volumeInterface);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("init failed, SLPlayerObject GetInterface Volume, %d", result);
        return AUDIO_ERROR_INIT;
    }

    result = (*m_playInterface)->SetPlayState(m_playInterface, SL_PLAYSTATE_PLAYING);

    return 0;
}

int OpenSLAudio::play(void* data, uint32_t size)
{
    LOGD("OpenSLAudio play, data=%d, size=%d", data, size);
    if(size <= 0)
        return AUDIO_ERROR_PLAY;

    SLresult result;
    result = (*m_bufferQueue)->Enqueue(m_bufferQueue, data, size);
    if(SL_RESULT_SUCCESS != result)
    {
        LOGW("OpenSLAudio play error");
        return AUDIO_ERROR_PLAY;
    }
    return 0;
}

void OpenSLAudio::playerCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context)
{
    LOGD("OpenSLAudio playerCallback");
}