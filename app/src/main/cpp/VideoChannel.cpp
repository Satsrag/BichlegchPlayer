#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#include "VideoChannel.h"
#include "Log.h"

VideoChannel::VideoChannel(int streamIndex, AVCodecContext *decoderContext) : BaseChannel(
        streamIndex, decoderContext) {

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
        if (mRenderCallback) {
            mRenderCallback(dstFrame[0], mDecoderContext->width, mDecoderContext->height,
                            linesSizes[0]);
        }
        av_frame_free(&frame);
    }
    av_frame_free(&frame);
    av_freep(dstFrame);
    sws_freeContext(swsContext);
}

VideoChannel::~VideoChannel() {

}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    mRenderCallback = renderCallback;
}

#pragma clang diagnostic pop