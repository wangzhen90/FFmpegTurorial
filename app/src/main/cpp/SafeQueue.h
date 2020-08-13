//
// Created by wangzhen on 2020/8/8.
//

#ifndef FFMPEGTURORIAL_SAFEQUEUE_H
#define FFMPEGTURORIAL_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>

class SafeQueue {
    typedef void (*ReleaseCallback)(T *);

    typedef void (*SyncHandle)(queue<T> &);

private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue<T> q;
    int isWork;
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void push(const T new_value) {
        pthread_mutex_lock(&mutex);
        if (isWork) {
            q.push(new_value);
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }

    int pop(T &value) {
        pthread_mutex_lock(&mutex);
        while (isWork && q.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        int isSuccess = 0;
        if (!q.empty()) {
            value = q.front();
            q.pop();
            isSuccess = 1;
        }
        pthread_mutex_unlock(&mutex);
        return isSuccess;
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->isWork = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        //同步代码块 当我们调用sync方法的时候，能够保证是在同步块中操作 queue 队列
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            releaseCallback(&value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback r) {
        releaseCallback = r;
    }

    void setSyncHandle(SyncHandle s) {
        syncHandle = s;
    }
};

#endif //FFMPEGTURORIAL_SAFEQUEUE_H
