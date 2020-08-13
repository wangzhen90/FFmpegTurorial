//
// Created by wangzhen on 2020/8/8.
//

#ifndef FFMPEGTURORIAL_BASECHANNEL_H
#define FFMPEGTURORIAL_BASECHANNEL_H

#include "SafeQueue.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

/**
 * channel 的职责就是将 AVPacket 解码后生成 AVFrame
 */
class BaseChannel {
public:
    BaseChannel(
            int id,
            AVCodecContext *avCodecContext,
            AVRational time_base
    ) : id(id), avCodecContext(avCodecContext),time_base(time_base) {
        packets.setReleaseCallback(BaseChannel::releaseAvPacket);
    }

    // 析构函数一般都是虚函数,释放先执行子类再执行父类
    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    // channel index，用来判断是音频流还是视频流
    int id;
    // 原始编码数据包队列
    SafeQueue<AVPacket *> packets;
    // 解码数据包队列
    SafeQueue<AVFrame *> frames;
    bool isPlaying;
    // 解码上下文
    AVCodecContext *avCodecContext;
    // 时间单位，用于计算 relativeTime，ffmpeg 会告诉你播放当前数据包相对于开始播放有多少个单位的时间
    AVRational time_base;
    // 播放当前数据包相对于开始播放的时间，单位：秒，最终用于音视频同步
    double relativeTime;

    /**
     * 释放 AVPacket
     * @param packet
     */
    static void releaseAvPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            // 指针的指针能够修改传递进来的指针的指向，这样就可以释放掉这个指针了
            *packet = 0;
        }
    }

    static void releaseAvFrame(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    //纯虚方法 相当于 抽象方法
    virtual void play() = 0;
};


#endif //FFMPEGTURORIAL_BASECHANNEL_H
