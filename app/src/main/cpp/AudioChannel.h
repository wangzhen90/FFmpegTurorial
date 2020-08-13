//
// Created by wangzhen on 2020/8/9.
//

#ifndef FFMPEGTURORIAL_AUDIOCHANNEL_H
#define FFMPEGTURORIAL_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
extern  "C"{
#include <libswresample/swresample.h>
}
class AudioChannel: public BaseChannel {
public:
    AudioChannel(int id,AVCodecContext *avCodecContext,AVRational time_base);
    ~AudioChannel();

    void play();

    void decode();

    void _play();

    int getPcmSize();

public:
    uint8_t *data = 0;
    // 声道数
    int out_channels;
    //
    int out_samplesize;
    // 采样率
    int out_sample_rate;
private:
    // 解码线程
    pthread_t  pid_audio_decode{};
    // 播放线程
    pthread_t  pid_audio_play{};

    /**
     * OpenSL ES
     */
    // 引擎
    SLObjectItf engineObject = 0;
    // 引擎接口,引擎相关的方法都在这个接口中
    SLEngineItf engineInterface = 0;
    // 混音器，可以实现不同的音效
    SLObjectItf outputMixObject = 0;
    // 播放器
    SLObjectItf bqPlayerObject = 0;
    // 播放器接口
    SLPlayItf bqPlayerInterface = 0;
    //
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface =0;
    // 重采样上下文
    SwrContext *swrContext = 0;
};


#endif //FFMPEGTURORIAL_AUDIOCHANNEL_H
