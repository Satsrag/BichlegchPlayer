//
// Created by Sachrag Zaanzab Borzood on 2020-01-28.
//

#ifndef BICHLEGCHPLAYER_PLAYER_H
#define BICHLEGCHPLAYER_PLAYER_H

#include <cstring>
#include <pthread.h>
#include "JNICallback.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
};

class Player {
public:
    Player(const char *dataSource, JNICallback *jniCallback);

    ~Player();

    void setRenderCallback(RenderCallback renderCallback);

    void prepare();

    void prepare_();

    void start();

    void start_();

private:
    char *mDataSource = NULL;
    int mPlaying = 0;
    JNICallback *mJNICallback = NULL;
    pthread_t mPrepareThread;
    pthread_t mStartThread;
    AVFormatContext *mAVFormatContext = NULL;

    VideoChannel *mVideoChannel = NULL;
    AudioChannel *mAudioChannel = NULL;

    AVCodecContext *mVideoDecoderContext = NULL;
    AVCodecContext *mAudioDecoderContext = NULL;

    RenderCallback mRenderCallback;
};

#endif //BICHLEGCHPLAYER_PLAYER_H
