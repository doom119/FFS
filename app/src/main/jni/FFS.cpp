//
// Created by Doom119 on 16/2/27.
//
#include "FFS.h"
#include "FFmpegPlayer.h"
#include "SDLRenderer.h"
#include "OpenSLAudio.h"
#include <unistd.h>

JavaVM *globalVM = 0;

using namespace FFS;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    LOGD("JNI_OnLoad");
    globalVM = vm;

    return JNI_VERSION_1_4;
}

void* getPlayerPtr(JNIEnv* env, jclass cls)
{
    jfieldID field = env->GetStaticFieldID(cls, "nativePlayerPtr", "J");
    jlong ptr = env->GetStaticLongField(cls, field);
    LOGD("nativePlayerPtr=%ld", ptr);
    return (void*)ptr;
}

void setPlayerPtr(JNIEnv* env, jclass cls, jlong ptr)
{
    jfieldID field = env->GetStaticFieldID(cls, "nativePlayerPtr", "J");
    env->SetStaticLongField(cls, field, ptr);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_nativeInit(JNIEnv *env, jclass cls, jstring renderCls)
{
    LOGD("nativeInit, tid=%d", gettid());
    const char* renderClassPath = env->GetStringUTFChars(renderCls, NULL);
    IPlayer *pPlayer = new FFmpegPlayer(renderClassPath);
    IRenderer *pRenderer = new SDLRenderer(renderClassPath);
    env->ReleaseStringUTFChars(renderCls, renderClassPath);

    IAudio *pAudio = new OpenSLAudio();

    void* ptrPlayer = getPlayerPtr(env, cls);
    if(NULL != ptrPlayer)
        delete ptrPlayer;
    setPlayerPtr(env, cls, (jlong)pPlayer);

    return pPlayer->init(pRenderer, pAudio);
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_nativeOpen(JNIEnv *env, jclass cls, jstring filename)
{
    IPlayer* ptrPlayer = (IPlayer*)getPlayerPtr(env, cls);
    if(NULL == ptrPlayer)
        return PLAYER_ERROR_OPEN;

    const char* file = env->GetStringUTFChars(filename, NULL);
    int ret = ptrPlayer->open(file);
    env->ReleaseStringUTFChars(filename, file);

    return ret;
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_nativePlay(JNIEnv *env, jclass cls)
{
    IPlayer* player = (IPlayer*)getPlayerPtr(env, cls);
    player->play();

    return 0;
}

JNIEXPORT jint JNICALL
Java_com_doom119_ffs_FFS_nativeClose(JNIEnv *env, jclass cls)
{
    void* ptrPlayer = getPlayerPtr(env, cls);

    delete (void*)ptrPlayer;
}

