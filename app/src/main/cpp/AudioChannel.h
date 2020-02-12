//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#ifndef BICHLEGCHPLAYER_AUDIOCHANNEL_H
#define BICHLEGCHPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel {

public:
    uint8_t *mAudioBuffer = NULL;

    AudioChannel(int streamIndex, AVCodecContext *decoderContext, AVRational timeBase);

    ~AudioChannel();

    void playThread();

    int getPCM();

private:
    SLObjectItf mEngine = NULL;
    SLEngineItf mEngineInterface = NULL;
    SLObjectItf mOutputMix = NULL;
    SLEnvironmentalReverbItf mOutputMixInterface = NULL;
    SLObjectItf mPlayerObject = NULL;
    SLPlayItf mPlayerInterface = NULL;
    SLAndroidSimpleBufferQueueItf mBufferQueueInterface = NULL;

    SwrContext *mSwrContext = NULL;

    const int mOutSampleRate = 44100;
    int mOutChannelCount;
    int mOutSampleBytes;
};

#endif //BICHLEGCHPLAYER_AUDIOCHANNEL_H
