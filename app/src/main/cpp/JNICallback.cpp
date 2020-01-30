//
// Created by Sachrag Zaanzab Borzood on 2020-01-28.
//
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"

#include "JNICallback.h"

JNICallback::JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance) {
    mJavaVM = javaVm;
    mEnv = env;
    mInstance = env->NewGlobalRef(instance);

    jclass bichlegchPlayerClass = mEnv->GetObjectClass(instance);
    mOnPreparedJMethod = mEnv->GetMethodID(bichlegchPlayerClass, "onPrepared", "()V");
    mOnErrorActionJMethod = mEnv->GetMethodID(bichlegchPlayerClass, "onErrorActions", "(I)V");
}

JNICallback::~JNICallback() {
    mJavaVM = NULL;
    mEnv->DeleteGlobalRef(mInstance);
    mInstance = NULL;
    // todo delete this
//    delete mOnPreparedJMethod;
//    delete mOnErrorActionJMethod;
    mEnv = NULL;
}

void JNICallback::onPrepared(int threadMode) {
    if (threadMode == THREAD_MAIN) {
        mEnv->CallVoidMethod(mInstance, mOnPreparedJMethod);
    } else {
        JNIEnv *env;
        jint ret = mJavaVM->AttachCurrentThread(&env, NULL);
        if (ret != JNI_OK) {
            return;
        }
        env->CallVoidMethod(mInstance, mOnPreparedJMethod);
        mJavaVM->DetachCurrentThread();
    }
}

void JNICallback::onErrorAction(int threadMode, int errorMode) {
    if (threadMode == THREAD_MAIN) {
        mEnv->CallVoidMethod(mInstance, mOnErrorActionJMethod, errorMode);
    } else {
        JNIEnv *env = NULL;
        jint ret = mJavaVM->AttachCurrentThread(&env, NULL);
        if (ret != JNI_OK) {
            return;
        }
        env->CallVoidMethod(mInstance, mOnErrorActionJMethod, errorMode);
        mJavaVM->DetachCurrentThread();
    }
}

#pragma clang diagnostic pop