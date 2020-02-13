//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#ifndef BICHLEGCHPLAYER_LOG_H
#define BICHLEGCHPLAYER_LOG_H

#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"zuga.com.bichlegchplayer", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"zuga.com.bichlegchplayer", __VA_ARGS__)

#endif //BICHLEGCHPLAYER_LOG_H
