//
// Created by Sachrag Zaanzab Borzood on 2020-01-28.
//

#ifndef BICHLEGCHPLAYER_JNICALLBACK_H
#define BICHLEGCHPLAYER_JNICALLBACK_H

#include <jni.h>
#include "Macro.h"

class JNICallback {
public:
    JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance);

    ~JNICallback();

    void onPrepared(int threadMode);

    void onErrorAction(int threadMode, int errorMode);

private:
    JavaVM *mJavaVM;
    JNIEnv *mEnv;
    jobject mInstance;

    jmethodID mOnPreparedJMethod;
    jmethodID mOnErrorActionJMethod;
};

#endif //BICHLEGCHPLAYER_JNICALLBACK_H