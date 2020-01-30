//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int streamIndex, AVCodecContext *decoderContext) : BaseChannel(
        streamIndex, decoderContext) {
    mOutSampleBytes = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    mOutChannelCount = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int audioBufferSize = mOutChannelCount * mOutSampleBytes * mOutSampleRate;
    mAudioBuffer = static_cast<uint8_t *>(malloc(static_cast<size_t>(audioBufferSize)));
    memset(mAudioBuffer, 0, static_cast<size_t>(audioBufferSize));

    mSwrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, mOutSampleRate,
                                     mDecoderContext->channel_layout, mDecoderContext->sample_fmt,
                                     mDecoderContext->sample_rate, 0, NULL);
    swr_init(mSwrContext);
}

AudioChannel::~AudioChannel() {
    swr_free(&mSwrContext);
    free(mAudioBuffer);
    mAudioBuffer = NULL;
    // todo release audio pointer
}

void bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    auto *audioChannel = static_cast<AudioChannel *>(pContext);
    int pcmSize = audioChannel->getPCM();
    (*caller)->Enqueue(caller, audioChannel->mAudioBuffer, static_cast<SLuint32>(pcmSize));
}

void AudioChannel::playThread() {
    SLresult ret = slCreateEngine(&mEngine, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mEngine)->Realize(mEngine, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mEngine)->GetInterface(mEngine, SL_IID_ENGINE, &mEngineInterface);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mEngineInterface)->CreateOutputMix(mEngineInterface, &mOutputMix, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mOutputMix)->Realize(mOutputMix, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
//    ret = (*mOutputMix)->GetInterface(mOutputMix, SL_IID_ENVIRONMENTALREVERB, &mOutputMixInterface);
//    if (SL_RESULT_SUCCESS == ret) {
//        //  设置混响 ： 默认。
////      SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
////      SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
//        const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
//        (*mOutputMixInterface)->SetEnvironmentalReverbProperties(mOutputMixInterface, &settings);
//    }

    // 创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    // pcm数据格式
    // SL_DATAFORMAT_PCM：数据格式为pcm格式
    // 2：双声道
    // SL_SAMPLINGRATE_44_1：采样率为44100（44.1赫兹 应用最广的，兼容性最好的）
    // SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit （16位）(2个字节)
    // SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit （16位）（2个字节）
    // SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）  （双声道 立体声的效果）
    // SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM pcmFormat = {
            SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSource = {&simpleBufferQueue, &pcmFormat};
    SLDataLocator_OutputMix outputMixLocator = {SL_DATALOCATOR_OUTPUTMIX, mOutputMix};
    SLDataSink audioSink = {&outputMixLocator, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    ret = (*mEngineInterface)->CreateAudioPlayer(mEngineInterface, &mPlayerObject, &audioSource,
                                                 &audioSink, 1, ids, req);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mPlayerObject)->Realize(mPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_PLAY, &mPlayerInterface);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_BUFFERQUEUE, &mBufferQueueInterface);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mBufferQueueInterface)->RegisterCallback(mBufferQueueInterface, bufferQueueCallback,
                                                     this);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    ret = (*mPlayerInterface)->SetPlayState(mPlayerInterface, SL_PLAYSTATE_PLAYING);
    if (SL_RESULT_SUCCESS != ret) {
        return;
    }
    bufferQueueCallback(mBufferQueueInterface, this);
}

int AudioChannel::getPCM() {
    int pcmSize = 0;
    AVFrame *frame = NULL;
    while (mPlaying) {
        int ret = mFrames->pop(frame);
        if (!mPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        int outSampleNumber = static_cast<int>(av_rescale_rnd(
                swr_get_delay(mSwrContext, frame->sample_rate) +
                frame->nb_samples, mOutSampleRate,
                frame->sample_rate,
                AV_ROUND_UP));
        ret = swr_convert(mSwrContext, &mAudioBuffer, outSampleNumber,
                          (const uint8_t **) frame->data,
                          frame->nb_samples);
        if (ret < 0) {
            break;
        }
        pcmSize = ret * mOutSampleBytes * mOutChannelCount;
        break;
    }
    av_frame_free(&frame);
    return pcmSize;
}
