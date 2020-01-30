#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by Sachrag Zaanzab Borzood on 2020-01-28.
//

#include "Player.h"
#include "Log.h"

Player::Player(const char *dataSource, JNICallback *jniCallback) {
    mDataSource = new char[strlen(dataSource) + 1];
    strcpy(mDataSource, dataSource);
    mJNICallback = jniCallback;
}

Player::~Player() {
    delete[] mDataSource;
    mJNICallback = NULL;
    delete mVideoChannel;
    delete mAudioChannel;
    if (mVideoDecoderContext) {
        avcodec_free_context(&mVideoDecoderContext);
    }
    if (mAudioDecoderContext) {
        avcodec_free_context(&mAudioDecoderContext);
    }
    if (mAVFormatContext) {
        avformat_close_input(&mAVFormatContext);
    }
}

void *prepareThread(void *object) {
    auto *player = static_cast<Player *>(object);
    player->prepare_();
    return NULL;
}

void *startThread(void *object) {
    auto *player = static_cast<Player *>(object);
    player->start_();
    return NULL;
}

void Player::prepare() {
    pthread_create(&mPrepareThread, NULL, &prepareThread, this);
}

void Player::prepare_() {
    mAVFormatContext = avformat_alloc_context();

    AVDictionary *dictionary = NULL;
    av_dict_set(&dictionary, "timeout", "5000000", 0);
    int ret = avformat_open_input(&mAVFormatContext, mDataSource, NULL, &dictionary);
    av_dict_free(&dictionary); // 释放字典
    if (ret) {
        if (mJNICallback) {
            mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    ret = avformat_find_stream_info(mAVFormatContext, NULL);
    if (ret < 0) {
        if (mJNICallback) {
            mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    for (int streamIndex = 0; streamIndex < mAVFormatContext->nb_streams; ++streamIndex) {
        AVStream *avStream = mAVFormatContext->streams[streamIndex];
        AVCodecParameters *decoderParam = avStream->codecpar;
        AVCodec *decoder = avcodec_find_decoder(decoderParam->codec_id);
        if (!decoder) {
            if (mJNICallback != NULL) {
                mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        AVCodecContext *decoderContext = avcodec_alloc_context3(decoder);
        if (!decoderContext) {
            if (mJNICallback) {
                mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        ret = avcodec_parameters_to_context(decoderContext, decoderParam);
        if (ret < 0) {
            if (mJNICallback != NULL) {
                mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        ret = avcodec_open2(decoderContext, decoder, NULL);
        if (ret) {
            if (mJNICallback != NULL) {
                mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        switch (decoderParam->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                mVideoDecoderContext = decoderContext;
                mVideoChannel = new VideoChannel(streamIndex, decoderContext);
                mVideoChannel->setRenderCallback(mRenderCallback);
                break;
            case AVMEDIA_TYPE_AUDIO:
                mAudioDecoderContext = decoderContext;
                mAudioChannel = new AudioChannel(streamIndex, decoderContext);
                break;
            default:
                avcodec_free_context(&decoderContext);
                break;
        }
    }
    if (!mVideoChannel && !mAudioChannel) {
        if (mJNICallback != NULL) {
            mJNICallback->onErrorAction(THREAD_CHILD, FFMPEG_NOMEDIA);
        }
        return;
    }
    if (mJNICallback != NULL) {
        mJNICallback->onPrepared(THREAD_CHILD);
    }
}

void Player::start() {
    mPlaying = 1;
    if (mAudioChannel) {
        mAudioChannel->start();
    }
    if (mVideoChannel) {
        mVideoChannel->start();
    }
    pthread_create(&mStartThread, NULL, &startThread, this);
}

void Player::start_() {
    const unsigned int sleepTime = 10000;
    const long maxPacketsSize = 1000;
    while (mPlaying) {
        if (mVideoChannel && mVideoChannel->mPackets->size() > maxPacketsSize) {
            LOGD("video channel packets is full now sleep %d millisecond", sleepTime);
            av_usleep(sleepTime);
            continue;
        }
        if (mAudioChannel && mAudioChannel->mPackets->size() > maxPacketsSize) {
            LOGD("audio channel packets is full now sleep %d millisecond", sleepTime);
            av_usleep(sleepTime);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(mAVFormatContext, packet);
        if (!ret) {
            if (mVideoChannel && mVideoChannel->mStreamIndex == packet->stream_index) {
                mVideoChannel->mPackets->push(packet);
            } else if (mAudioChannel && mAudioChannel->mStreamIndex == packet->stream_index) {
                mAudioChannel->mPackets->push(packet);
            } else {
                av_packet_free(&packet);
                continue;
            }
        } else if (AVERROR_EOF == ret) {
            // todo handle end
        } else {
            av_packet_free(&packet);
            break;
        }
    }
    mPlaying = 0;
    if (mVideoChannel) {
        mVideoChannel->stop();
    }
    if (mAudioChannel) {
        mAudioChannel->stop();
    }
}

void Player::setRenderCallback(RenderCallback renderCallback) {
    mRenderCallback = renderCallback;
}


#pragma clang diagnostic pop