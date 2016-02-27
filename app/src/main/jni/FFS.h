//
// Created by Doom119 on 16/2/16.
//

#ifndef FFS_FFS_H
#define FFS_FFS_H

#include <jni.h>
#include "Error.h"
#include "com_doom119_ffs_FFS.h"

#include <android/log.h>
#define LOG_TAG "FFS_native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern JavaVM *globalVM;

#endif //FFS_FFS_H
