//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#ifndef BICHLEGCHPLAYER_BASECHANNEL_H
#define BICHLEGCHPLAYER_BASECHANNEL_H

#include "SafeQueue.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

class BaseChannel {
private:
    pthread_t mDecodeThread;
    pthread_t mPlayThread;

protected:
    int mPlaying = 0;
    AVCodecContext *mDecoderContext = NULL;

public:
    int mStreamIndex;
    SafeQueue<AVPacket *> *mPackets = NULL;
    SafeQueue<AVFrame *> *mFrames = NULL;

    BaseChannel(int streamIndex, AVCodecContext *decoderContext);

    virtual ~BaseChannel();

    void start();

    void stop();

    void decodeThread();

    virtual void playThread() = 0;
};

#endif //BICHLEGCHPLAYER_BASECHANNEL_H
