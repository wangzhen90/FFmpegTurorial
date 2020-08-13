//
// Created by wangzhen on 2020/8/2.
//

#ifndef FFMPEGTURORIAL_MEDIABRIDGE_H
#define FFMPEGTURORIAL_MEDIABRIDGE_H


#include <jni.h>

class MediaBridge {
public:
    MediaBridge(JavaVM *vm,JNIEnv *env,jobject instance);
    ~MediaBridge();
   void onError(int thread,int errorCode);
   void onPrepare(int thread);

private:
    JavaVM *vm;
    JNIEnv  *env;
    jobject  instance;
    jmethodID onErrorId;
    jmethodID onPrepareId;
};


#endif //FFMPEGTURORIAL_MEDIABRIDGE_H
