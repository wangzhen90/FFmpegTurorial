//
// Created by wangzhen on 2020/8/2.
//

#include "MediaBridge.h"
#include "macro.h"

MediaBridge::MediaBridge(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm = vm;
    this->env = env;
    // 涉及到 jobject 的跨线程使用就需要创建全局引用
    this->instance = env->NewGlobalRef(instance);
    jclass clazz = env->GetObjectClass(instance);
    onErrorId = env->GetMethodID(clazz, "onError", "(I)V");
    onPrepareId = env->GetMethodID(clazz, "onPrepare", "()V");
}

MediaBridge::~MediaBridge() {
    env->DeleteGlobalRef(instance);
}

void MediaBridge::onError(int thread, int errorCode) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(instance, onErrorId, errorCode);
    } else if (thread == THREAD_CHILD) {
        JNIEnv *threadEnv;
        vm->AttachCurrentThread(&threadEnv, nullptr);
        threadEnv->CallVoidMethod(instance, onErrorId, errorCode);
        vm->DetachCurrentThread();
    }
}

void MediaBridge::onPrepare(int thread) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(instance, onPrepareId);
    } else {
        //子线程
        JNIEnv *threadEnv;
        //获得属于当前子线程的 jnienv
        vm->AttachCurrentThread(&threadEnv, nullptr);
        threadEnv->CallVoidMethod(instance, onPrepareId);
        vm->DetachCurrentThread();
    }
}
