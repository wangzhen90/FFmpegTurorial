//
// Created by wangzhen on 2020/8/2.
//

#include "MediaManager.h"
#include "macro.h"
#include <cstring>

void *prepareTask(void *args) {
    auto *mediaManager = static_cast<MediaManager *>(args);
    mediaManager->_prepare();
    return nullptr;
}

MediaManager::MediaManager(MediaBridge *bridge, const char *dataSource) {
    this->mediaBridge = bridge;
    // 防止外部 str 释放，要拷贝一份
    this->dataSource = new char[strlen(dataSource)];
    strcpy(this->dataSource, dataSource);
}

MediaManager::~MediaManager() {
    DELETE(dataSource)
    DELETE(mediaBridge)
}

void MediaManager::prepare() {
    LOGE("start prepare");
    pthread_create(&pid_prepare, nullptr, prepareTask, this);
}

void MediaManager::_prepare() {
    // ffmpeg 如果需要联网需要先出示话网络
    avformat_network_init();

    // 1.打开多媒体流（本地文件/网络地址）
    formatContext = nullptr;
    int result = avformat_open_input(&formatContext, dataSource, nullptr, nullptr);
    if (result != 0) {
        LOGE("打开媒体失败:%s", av_err2str(result));
        mediaBridge->onError(THREAD_CHILD, result);
        return;
    }

    // 2.查找多媒体流中包含的流，比如音频流，视频流
    result = avformat_find_stream_info(formatContext, nullptr);
    if (result < 0) {
        LOGE("查找流失败:%s", av_err2str(result));
        mediaBridge->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    // 3.处理包含的流
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        // 解码这个流的相关信息
        AVCodecParameters *codecpar = stream->codecpar;
        // 查找该流的解码器
        AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
        if (decoder == nullptr) {
            LOGE("查找解码器失败");
            mediaBridge->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        // 获取解码器上下文
        AVCodecContext *decoderContext = avcodec_alloc_context3(decoder);
        if (decoderContext == nullptr) {
            LOGE("创建解码上下文失败");
            mediaBridge->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }

        // 给解码器上下文设置参数
        result = avcodec_parameters_to_context(decoderContext, codecpar);
        if (result < 0) {
            LOGE("设置解码上下文参数失败:%s", av_err2str(result));
            mediaBridge->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        // 打开解码器
        result = avcodec_open2(decoderContext, decoder, nullptr);
        if (result != 0) {
            LOGE("打开解码器失败:%s", av_err2str(result));
            mediaBridge->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        AVRational time_base = stream->time_base;
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, decoderContext, time_base);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // 帧率：单位时间内 需要显示多少个图像
            AVRational frame_rate = stream->avg_frame_rate;
            int fps = av_q2d(frame_rate);
            videoChannel = new VideoChannel(i, decoderContext, time_base, fps);
            videoChannel->setRenderFrameCallback(callback);
        }
        if (!videoChannel) {
            LOGE("没有音视频");
            mediaBridge->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
            return;
        }
    }
    // 准备完成
    mediaBridge->onPrepare(THREAD_CHILD);
}

void *playTask(void *args) {
    auto mediaManager = static_cast<MediaManager *>(args);
    mediaManager->_start();
    return nullptr;
}

void MediaManager::start() {
    this->isPlaying = true;
    if(audioChannel){
        audioChannel->play();
    }
    if (videoChannel) {
        LOGE("videoChannel start play");
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }
    pthread_create(&pid_play, nullptr, playTask, this);
}

/**
 * 读取 AVPackage
 */
void MediaManager::_start() {
    int result = 0;
    while (isPlaying) {
        AVPacket *packet = av_packet_alloc();
        result = av_read_frame(formatContext, packet);
        if (result == 0) {
            if (videoChannel && packet->stream_index == videoChannel->id) {
                videoChannel->packets.push(packet);
            }else if (audioChannel && packet->stream_index == audioChannel->id){
                audioChannel->packets.push(packet);
            }
        } else if (result == AVERROR_EOF) {
            //读取完成 但是可能还没播放完
        } else {
            break;
        }
    }
}

void MediaManager::setRenderFrameCallback(RenderFrameCallback new_callback) {
    this->callback = new_callback;
}
