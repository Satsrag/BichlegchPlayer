#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#include "VideoChannel.h"
#include "Log.h"

VideoChannel::VideoChannel(int streamIndex, AVCodecContext *decoderContext, AVRational timeBase,
                           int fps)
        : BaseChannel(streamIndex, decoderContext, timeBase) {
    mFps = fps;
}

void VideoChannel::playThread() {
    SwsContext *swsContext = sws_getContext(mDecoderContext->width, mDecoderContext->height,
                                            mDecoderContext->pix_fmt,
                                            mDecoderContext->width, mDecoderContext->height,
                                            AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR, NULL, NULL, NULL
    );
    uint8_t *dstFrame[4];
    int linesSizes[4];
    // todo align what it is
    av_image_alloc(dstFrame, linesSizes, mDecoderContext->width, mDecoderContext->height,
                   AV_PIX_FMT_RGBA, 1);

    AVFrame *frame = NULL;

    while (mPlaying) {
        int ret = mFrames->pop(frame);
        if (!mPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, mDecoderContext->height, dstFrame,
                  linesSizes);
        double frameDelayTime = frame->repeat_pict + 1.0 / mFps;
        mTimestamp = frame->best_effort_timestamp;
        double standardTimestampSecond = mStandardTimestampChannel->mTimestamp *
                                         av_q2d(mStandardTimestampChannel->mTimeBase);
        double videoTimestampSecond = mTimestamp * av_q2d(mTimeBase);
        double deltaTimestampSecond = videoTimestampSecond - standardTimestampSecond;
        if (deltaTimestampSecond > 0) {
            // video is faster than standard
            if (deltaTimestampSecond > 1) {
                av_usleep(static_cast<unsigned int>(frameDelayTime * 2 * 1000000));
            } else {
                av_usleep(static_cast<unsigned int>(deltaTimestampSecond * 1000000));
            }
        } else if (deltaTimestampSecond < 0) {
            // video is slow that standard
            av_frame_free(&frame);
            continue;
        }
        if (mRenderCallback) {
            mRenderCallback(dstFrame[0], mDecoderContext->width, mDecoderContext->height,
                            linesSizes[0], mRenderCallbackObject);
        }
        av_frame_free(&frame);
    }
    av_frame_free(&frame);
    av_freep(dstFrame);
    sws_freeContext(swsContext);
}

VideoChannel::~VideoChannel() {
    mRenderCallback = NULL;
    mRenderCallbackObject = NULL;
    mStandardTimestampChannel = NULL;
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback, void *object) {
    mRenderCallback = renderCallback;
    mRenderCallbackObject = object;
}

void VideoChannel::setStandardTimestampChannel(BaseChannel *standardTimestampChannel) {
    mStandardTimestampChannel = standardTimestampChannel;
}

#pragma clang diagnostic pop