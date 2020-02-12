//
// Created by Sachrag Zaanzab Borzood on 2020-01-29.
//

#ifndef BICHLEGCHPLAYER_VIDEOCHANNEL_H
#define BICHLEGCHPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

extern "C" {
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};


class VideoChannel : public BaseChannel {

public:

    typedef void (*RenderCallback)(uint8_t *, int, int, int, void *);

    VideoChannel(int streamIndex, AVCodecContext *decoderContext);

    ~VideoChannel();

    /**
     * @param renderCallback uint8_t * render rgba data
     *                       int width
     *                       int height
     *                       int lineSize
     */
    void setRenderCallback(RenderCallback renderCallback, void *object);

    void playThread();

private:
    RenderCallback mRenderCallback = NULL;
    void *mRenderCallbackObject = NULL;
};


#endif //BICHLEGCHPLAYER_VIDEOCHANNEL_H
