//
// Created by wangzhen on 2020/8/8.
//

#ifndef FFMPEGTURORIAL_VIDEOCHANNEL_H
#define FFMPEGTURORIAL_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libswscale/swscale.h>
};

typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, AVCodecContext *avCodecContext, AVRational time_base, int fps);

    ~VideoChannel();

    // 解码+播放
    void play();

    // 解码：AVPacket -> AVFrame
    void decode();

    // 播放：处理 AVFrame，获取其中的图形数据
    void render();

    // 设置播放回调
    void setRenderFrameCallback(RenderFrameCallback callback);

    void setAudioChannel(AudioChannel* audioChannel);

private:
    // 解码线程
    pthread_t pid_decode;
    // 播放线程
    pthread_t pid_render;
    // 转换图像上下文
    SwsContext *swsContext = 0;
    // 用于刷新 window 的回调
    RenderFrameCallback callback;
    // 帧率：用于计算单位时间内需要显示多少个图像，如果不以 fps 显示图像，接受多少包就播放多少包就会显得不流畅
    int fps;
    AudioChannel *audioChannel = 0;
};


#endif //FFMPEGTURORIAL_VIDEOCHANNEL_H
