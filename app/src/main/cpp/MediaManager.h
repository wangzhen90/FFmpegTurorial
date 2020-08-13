//
// Created by wangzhen on 2020/8/2.
//

#ifndef FFMPEGTURORIAL_MEDIAMANAGER_H
#define FFMPEGTURORIAL_MEDIAMANAGER_H

#include "MediaBridge.h"
#include <pthread.h>
#include "VideoChannel.h"
#include "AudioChannel.h"
extern "C" {
#include <libavformat/avformat.h>
}
class MediaManager {
public:
    MediaManager(MediaBridge *bridge, const char *dataSource);

    ~MediaManager();

    void prepare();
    void _prepare();

    void start();
    void _start();

    void setRenderFrameCallback(RenderFrameCallback callback);
private:
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_play;
    // AVFormatContext  包含了 视频的 信息(宽、高、比特率、帧率等)
    AVFormatContext *formatContext;
    MediaBridge *mediaBridge;
    // 视频处理
    VideoChannel *videoChannel;
    // 音频处理
    AudioChannel *audioChannel;
    RenderFrameCallback callback;
    bool isPlaying{};
};

#endif //FFMPEGTURORIAL_MEDIAMANAGER_H
