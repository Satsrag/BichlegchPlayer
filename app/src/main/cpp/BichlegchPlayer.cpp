
#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <map>

#include "Player.h"
#include "Log.h"


extern "C" {
#include <libavutil/avutil.h>
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"

JavaVM *mJavaVM = NULL;

std::map<int, Player *> *mPlayerMap = NULL;
std::map<int, JNICallback *> *mJniCallback = NULL;
std::map<Player *, pthread_mutex_t *> *mMutexMap = NULL;
std::map<Player *, ANativeWindow *> *mNativeWindowMap = NULL;


template<class T>
struct ReleaseCallbackWrapper {
    typedef void (*ReleaseCallback)(T **);
};


template<typename Key, typename Value>
void putMap(std::map<Key, Value *> **map, Key key, Value *value) {
    if (*map == NULL) {
        *map = new std::map<Key, Value *>;
    }
    (**map)[key] = value;
}

template<typename Key, typename Value>
void removeMap(std::map<Key, Value *> *map, Key key,
               typename ReleaseCallbackWrapper<Value>::ReleaseCallback releaseCallback) {
    if (map == NULL) {
        return;
    }
    Value *value = (*map)[key];
    if (value) {
        map->erase(key);
    }
    releaseCallback(&value);
    if (map->empty()) {
        delete map;
    }
}

template<typename Key, typename Value>
Value *getMap(std::map<Key, Value *> *map, Key key) {
    if (map && map->count(key) > 0) {
        Value *x = (*map)[key];
        return x;
    }
    return NULL;
}

template<typename Value>
void release(Value **pValue) {
    if (*pValue) {
        delete *pValue;
        *pValue = NULL;
    }
}

void releaseNativeWindow(ANativeWindow **pNativeWindow) {
    if (*pNativeWindow) {
        ANativeWindow_release(*pNativeWindow);
        *pNativeWindow = NULL;
    }
}

int getHashCode(JNIEnv *env, jobject object) {
    jclass pJclass = env->GetObjectClass(object);
    jmethodID pId = env->GetMethodID(pJclass, "hashCode", "()I");
    jint hashCode = env->CallIntMethod(object, pId);
    return hashCode;
}

void renderCallback(uint8_t *frameData, int width, int height, int lineSize, Player *player) {
    pthread_mutex_t *pMutex = getMap(mMutexMap, player);
    if (pMutex == NULL) {
        return;
    }
    pthread_mutex_lock(pMutex);

    ANativeWindow *pNativeWindow = getMap(mNativeWindowMap, player);
    if (!pNativeWindow) {
        pthread_mutex_unlock(pMutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(pNativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;
    //todo inOutDirtyBounds what it is
    int32_t ret = ANativeWindow_lock(pNativeWindow, &buffer, NULL);
    if (ret) {
        removeMap<Player *, ANativeWindow>(mNativeWindowMap, player, releaseNativeWindow);
        pthread_mutex_unlock(pMutex);
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
    ANativeWindow_unlockAndPost(pNativeWindow);
    pthread_mutex_unlock(pMutex);
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
    auto *pPlayer = new Player(dataSource, jniCallback);

    int thizHashCode = getHashCode(env, thiz);
    putMap(&mMutexMap, pPlayer, new pthread_mutex_t(PTHREAD_MUTEX_INITIALIZER));
    putMap(&mJniCallback, thizHashCode, jniCallback);
    putMap(&mPlayerMap, thizHashCode, pPlayer);

    pPlayer->setRenderCallback(renderCallback);
    pPlayer->prepare();
    env->ReleaseStringUTFChars(data_source, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativeStart(JNIEnv *env, jobject thiz) {
    Player *pPlayer = getMap(mPlayerMap, getHashCode(env, thiz));
    if (pPlayer == NULL) {
        return;
    }
    pPlayer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativeSetSurface(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    Player *pPlayer = getMap(mPlayerMap, getHashCode(env, thiz));
    if (pPlayer == NULL) {
        return;
    }
    pthread_mutex_t *pMutex = getMap(mMutexMap, pPlayer);
    if (pMutex == NULL) {
        return;
    }
    pthread_mutex_lock(pMutex);

    ANativeWindow *pNativeWindow = getMap(mNativeWindowMap, pPlayer);
    if (pNativeWindow) {
        removeMap<Player *, ANativeWindow>(mNativeWindowMap, pPlayer, &releaseNativeWindow);
    }
    pNativeWindow = ANativeWindow_fromSurface(env, surface);
    putMap(&mNativeWindowMap, pPlayer, pNativeWindow);
    pthread_mutex_unlock(pMutex);
}

#pragma clang diagnostic pop
extern "C"
JNIEXPORT void JNICALL
Java_zuga_com_bichlegchplayer_BichlegchPlayer_nativeRelease(JNIEnv *env, jobject thiz) {
    Player *pPlayer = getMap(mPlayerMap, getHashCode(env, thiz));
    if (pPlayer) {
        pPlayer->release();
        int thizHashCode = getHashCode(env, thiz);
        removeMap(mMutexMap, pPlayer, &release);
        removeMap(mNativeWindowMap, pPlayer, &releaseNativeWindow);
        removeMap(mJniCallback, thizHashCode, release);
        removeMap(mPlayerMap, thizHashCode, release);
    }
}