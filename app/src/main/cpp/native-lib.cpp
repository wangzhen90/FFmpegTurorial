#include <jni.h>
#include <string>
#include "MediaBridge.h"
#include "MediaManager.h"
#include "SafeQueue.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include <android/native_window_jni.h>

extern "C" {
#include "libavutil/avutil.h"
}
JavaVM *javaVm = nullptr;
MediaManager *mediaManager = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ANativeWindow *window = nullptr;

/**
 * 拿到 ffmpeg 当前版本
 * @return
 */
const char *getFFmpegVer() {
    return av_version_info();
}

int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_ffmpegturorial_SimplePlayer_getVersionNative(
        JNIEnv *env,
        jobject /* this */) {
    return env->NewStringUTF(getFFmpegVer());
}

void render(uint8_t *data, int lineszie, int w, int h) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    // 2.设置 buffer 的尺寸和格式
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    // 3. 填充数据
    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }

    // 填充rgb数据给dst_data
    auto dst_data = static_cast<uint8_t *>(window_buffer.bits);
    // stride：一行多少个数据（RGBA） *4
    int dst_linesize = window_buffer.stride * 4;
    // 一行一行的拷贝
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, data + i * lineszie, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_ffmpegturorial_SimplePlayer_nativeSetPlayView(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    pthread_mutex_lock(&mutex);
    if (window) {
        // 把老的释放
        ANativeWindow_release(window);
        window = nullptr;
    }
    // 1.通过 ANativeWindow_fromSurface 获取与 Surface 对应的 ANativeWindow 对象
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_ffmpegturorial_SimplePlayer_prepareFFmpegNative(JNIEnv *env, jobject thiz,
                                                                 jstring data_source) {
    const char *dataSource = env->GetStringUTFChars(data_source, nullptr);
    auto *bridge = new MediaBridge(javaVm, env, thiz);
    mediaManager = new MediaManager(bridge, dataSource);
    mediaManager->setRenderFrameCallback(render);
    mediaManager->prepare();
    env->ReleaseStringUTFChars(data_source, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_ffmpegturorial_SimplePlayer_nativeStartPlay(JNIEnv *env, jobject thiz) {
    mediaManager->start();
}
