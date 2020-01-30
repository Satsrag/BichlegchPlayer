
#include <jni.h>
#include <string>
#include "Player.h"
#include "Log.h"
#include <android/native_window_jni.h>

extern "C" {
#include <libavutil/avutil.h>
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"

Player *mPlayer = NULL;
JavaVM *mJavaVM = NULL;
pthread_mutex_t mMutex = PTHREAD_MUTEX_INITIALIZER;
ANativeWindow *mNativeWindow = NULL;

void renderCallback(uint8_t *frameData, int width, int height, int lineSize) {
    pthread_mutex_lock(&mMutex);
    if (!mNativeWindow) {
        pthread_mutex_unlock(&mMutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(mNativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;
    //todo inOutDirtyBounds what it is
    int32_t ret = ANativeWindow_lock(mNativeWindow, &buffer, NULL);
    if (ret) {
        ANativeWindow_release(mNativeWindow);
        mNativeWindow = NULL;
        pthread_mutex_unlock(&mMutex);
        return;
    }
    auto *dstData = static_cast<uint8_t *>(buffer.bits);
    int windowLineSize = buffer.stride * 4;
    for (int i = 0; i < buffer.height; ++i) {
        int startOffset = i * windowLineSize;
        // todo if we use lineSize
        memcpy(dstData + startOffset, frameData + startOffset,
               static_cast<size_t>(windowLineSize));
    }
    ANativeWindow_unlockAndPost(mNativeWindow);
    pthread_mutex_unlock(&mMutex);
}

int JNI_OnLoad(JavaVM *javaVm, void *pVoid) {
    mJavaVM = javaVm;
    return JNI_VERSION_1_6; // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
}

extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativePrepare(JNIEnv *env, jobject thiz,
                                                            jstring data_source) {
    auto *jniCallback = new JNICallback(mJavaVM, env, thiz);
    const char *dataSource = env->GetStringUTFChars(data_source, NULL);
    mPlayer = new Player(dataSource, jniCallback);
    mPlayer->setRenderCallback(renderCallback);
    mPlayer->prepare();
    env->ReleaseStringUTFChars(data_source, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativeStart(JNIEnv *env, jobject thiz) {
    mPlayer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativeSetSurface(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    pthread_mutex_lock(&mMutex);
    if (mNativeWindow) {
        ANativeWindow_release(mNativeWindow);
        mNativeWindow = NULL;
    }
    mNativeWindow = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mMutex);
}

#pragma clang diagnostic pop