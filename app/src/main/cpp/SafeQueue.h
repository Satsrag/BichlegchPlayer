//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#ifndef BICHLEGCHPLAYER_SAFEQUEUE_H
#define BICHLEGCHPLAYER_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

template<typename T>
class SafeQueue {

private:
    std::queue<T> mQueue;
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
    int mWorking = 0;
public:

    typedef void (*ReleaseCallback)(T *);

    ReleaseCallback mReleaseCallback;

    SafeQueue() {
        pthread_mutex_init(&mMutex, NULL);
        pthread_cond_init(&mCond, NULL);
    }

    ~SafeQueue() {
        pthread_cond_destroy(&mCond);
        pthread_mutex_destroy(&mMutex);
    }

    void push(T t) {
        pthread_mutex_lock(&mMutex);
        if (mWorking) {
            mQueue.push(t);
            pthread_cond_signal(&mCond);
        } else {
            if (mReleaseCallback) {
                mReleaseCallback(&t);
            }
        }
        pthread_mutex_unlock(&mMutex);
    }

    int pop(T &t) {
        int ret = 0;
        pthread_mutex_lock(&mMutex);
        while (mWorking && mQueue.empty()) {
            pthread_cond_wait(&mCond, &mMutex);
        }
        if (!mQueue.empty()) {
            t = mQueue.front();
            mQueue.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mMutex);
        return ret;
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        mReleaseCallback = releaseCallback;
    }

    void setWorking(int working) {
        pthread_mutex_lock(&mMutex);
        mWorking = working;
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    long size() {
        return mQueue.size();
    }

    void clear() {
        pthread_mutex_lock(&mMutex);
        unsigned long size = mQueue.size();
        for (int i = 0; i < size; ++i) {
            T &t = mQueue.front();
            if (mReleaseCallback) {
                mReleaseCallback(&t);
            }
            mQueue.pop();
        }
        pthread_mutex_unlock(&mMutex);
    }

};

#endif //BICHLEGCHPLAYER_SAFEQUEUE_H
