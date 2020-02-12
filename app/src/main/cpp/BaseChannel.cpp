#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#include "BaseChannel.h"
#include "Log.h"

void deletePacket(AVPacket **packet) {
    if (packet && *packet) {
        av_packet_free(packet);
        *packet = NULL;
    }
}

void deleteFrame(AVFrame **frame) {
    if (frame && *frame) {
        av_frame_free(frame);
        *frame = NULL;
    }
}

void *decodeThread(void *object) {
    auto *channel = static_cast<BaseChannel *>(object);
    channel->decodeThread();
    return NULL;
}

void *playThread(void *object) {
    auto *channel = static_cast<BaseChannel *>(object);
    channel->playThread();
    return NULL;
}

BaseChannel::BaseChannel(int streamIndex, AVCodecContext *decoderContext, AVRational timeBase) {
    mStreamIndex = streamIndex;
    mDecoderContext = decoderContext;
    mPackets = new SafeQueue<AVPacket *>;
    mFrames = new SafeQueue<AVFrame *>;
    mPackets->setReleaseCallback(deletePacket);
    mFrames->setReleaseCallback(deleteFrame);
    mTimeBase = timeBase;
}

BaseChannel::~BaseChannel() {
    mPlaying = 0;
    mPackets->clear();
    mFrames->clear();
    delete mPackets;
    delete mFrames;
    mDecoderContext = NULL;
    LOGE("BaseChannel destruct");
}

void BaseChannel::start() {
    mPlaying = 1;
    mPackets->setWorking(1);
    mFrames->setWorking(1);
    pthread_create(&mDecodeThread, NULL, ::decodeThread, this);
    pthread_create(&mPlayThread, NULL, ::playThread, this);
}

void BaseChannel::stop() {
    mPlaying = 0;
    mPackets->setWorking(0);
    pthread_join(mDecodeThread, NULL);
    mFrames->setWorking(0);
    pthread_join(mPlayThread, NULL);
}

void BaseChannel::decodeThread() {
    AVPacket *packet = NULL;
    const int maxFrameCount = 1000;
    const int sleepTime = 10 * 1000;
    while (mPlaying) {
        if (mPlaying && mFrames->size() > maxFrameCount) {
            av_usleep(sleepTime);
            continue;
        }
        int ret = mPackets->pop(packet);
        if (!mPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(mDecoderContext, packet);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret) {
            break;
        }
        av_packet_free(&packet);
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(mDecoderContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            av_frame_free(&frame);
            continue;
        } else if (ret) {
            av_frame_free(&frame);
            break;
        }
        mFrames->push(frame);
    }
    av_packet_free(&packet);
}

#pragma clang diagnostic pop