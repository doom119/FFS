//
// Created by Doom119 on 16/3/5.
//

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include "../FFS.h"

#ifndef FFS_THREAD_H
#define FFS_THREAD_H

namespace FFS
{
    typedef void* (*ENTRYPOINT)(void*);

    class Thread
    {
    public:
        Thread():m_nThreadId(0),m_bIsFinished(false)
        {

        }
        Thread(const char* threadName):m_strName(threadName), m_nThreadId(0),
                    m_bIsFinished(false)
        {

        }
        uint32_t start(ENTRYPOINT entry, void* args);
        void exit();
        virtual ~Thread(){};

    private:
        static void* run(void* args);

    private:
        const char* m_strName;
        long m_nThreadId;
        bool m_bIsFinished;
    };

    inline uint32_t Thread::start(ENTRYPOINT entry, void* args)
    {
        int ret = pthread_create(&m_nThreadId, NULL, entry, args);
        if(0 != ret)
        {
            LOGW("create thread %s failed, ret=%d", m_strName, ret);
        }

        return ret;
    }

    inline void Thread::exit()
    {
        m_bIsFinished = true;
    }

    inline void* Thread::run(void *args)
    {
//        LOGD("Thread run");
//        Thread* thread = (Thread*)args;
    }
};
#endif //FFS_THREAD_H
