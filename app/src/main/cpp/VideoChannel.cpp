//
// Created by wangzhen on 2020/8/8.
//

#include "VideoChannel.h"
#include "macro.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

void *decodeTask(void *args) {
    auto *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    return nullptr;
}

void *renderTask(void *args) {
    auto *channel = static_cast<VideoChannel *>(args);
    channel->render();
    return nullptr;
}

/**
 * 丢已经解码的图片
 */
void dropAvFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAvFrame(&frame);
        q.pop();
    }
}

VideoChannel::VideoChannel(
        int id,
        AVCodecContext *avCodecContext,
        AVRational time_base,
        int fps
) : BaseChannel(id, avCodecContext, time_base) {
    this->fps = fps;
    frames.setReleaseCallback(releaseAvFrame);
    frames.setSyncHandle(dropAvFrame);
}

VideoChannel::~VideoChannel() {

}

void VideoChannel::play() {
    isPlaying = true;
    packets.setWork(1);
    frames.setWork(1);
    // 1、解码
    pthread_create(&pid_decode, nullptr, decodeTask, this);
    // 2、播放
    pthread_create(&pid_render, nullptr, renderTask, this);
}

//解码
void VideoChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        // 从队列中取出一个数据包
        int result = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        // 取出失败
        if (!result) {
            continue;
        }
        // 把包丢给解码器
        result = avcodec_send_packet(avCodecContext, packet);
        // avcodec 处理完 packet 之后就可以将 packet 销毁了
        releaseAvPacket(&packet);
        // 重试
        if (result != 0) {
            break;
        }
        // 代表了一个图像 (将这个图像先输出来)
        AVFrame *frame = av_frame_alloc();
        // 从解码器中读取 解码后的数据包 AVFrame
        result = avcodec_receive_frame(avCodecContext, frame);
        // 需要更多的数据才能够进行解码
        if (result == AVERROR(EAGAIN)) {
            continue;
        } else if (result != 0) {
            break;
        }
        // 再开一个线程来播放 (为了保障流畅度)
        frames.push(frame);
    }
    releaseAvPacket(&packet);
}

//播放
void VideoChannel::render() {
    // 将图像转化为 RGBA 格式
    // 转换使用的是 swscale 这个库，先创建一个 swscale 的 context
    swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);
    //每一帧刷新的间隔 单位：秒
    double frame_period = 1.0 / fps;
    AVFrame *frame = nullptr;
    // 指针数组
    uint8_t *dst_data[4];
    int dst_linesize[4];
    // 由于 sws_scale 不接受二级指针的数据 buffer 数组，需要先申请一块内存，sws_scale 会向这块内存存数据
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);
    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        // src_linesize: 表示每一行存放的 字节长度
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data),
                  frame->linesize, 0,
                  avCodecContext->height,
                  dst_data,
                  dst_linesize);
#if 0
        // 用多种方式估算出的帧的时间戳
        double relativeTime = frame->best_effort_timestamp * av_q2d(time_base);
        // 额外的间隔时间: 这个图像必须延时多长
        double extra_delay = frame->repeat_pict / (2 * fps); // NOLINT(bugprone-integer-division)
        // 真实需要的间隔时间
        double delays = extra_delay + frame_period;
        if(!audioChannel){
            av_usleep(delays * 1000000);
        }else{
            if(relativeTime == 0){
                av_usleep(delays * 1000000);
            }else{
                // 比较音视频的时间差
                double diff = relativeTime - audioChannel->relativeTime;
                if(diff > 0){
                    // 视频比音频快
                    LOGE("视频快了：%lf",diff);
                    av_usleep((delays + diff) * 1000000);
                }else{
                    LOGE("音频快了：%lf",diff);
                    // 音频比视频快
                    // 视频包积压的太多了，这时候需要丢掉一些视频帧
                    if (fabs(diff) >= 0.05) {
                        releaseAvFrame(&frame);
                        // 主动丢包
                        frames.sync();
                        continue;
                    }else{
                        // 不睡了 快点赶上 音频
                    }
                }
            }
        }
#else
        // 单位是微秒
        av_usleep(frame_period * 1000000);
#endif
        // 回调出去进行播放
        callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        releaseAvFrame(&frame);
    }
    av_freep(&dst_data[0]);
    releaseAvFrame(&frame);
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}
