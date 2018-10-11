//
// Created by qq798 on 2018/8/8.
//

#ifndef MYFFMPEGPLAYER_WLXQUEUE_H
#define MYFFMPEGPLAYER_WLXQUEUE_H

#include <queue>

#include "WlxPlayerState.h"
#include "pthread.h"
#include "AndroidLog.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

class WlxQueue {
public:
    std::queue<AVPacket *> queue;
    int queueMaxSize = 1024;

    WlxPlayerState * playerState = NULL;
    pthread_mutex_t queueMutex;
    pthread_cond_t queueEmptyCond;
    pthread_cond_t queueFullCond;
    pthread_cond_t queuePushPauseCond;

    // 该变量会阻塞执行push的线程
    bool ispushPause = false;
public:
    WlxQueue(WlxPlayerState * playerState);
    ~WlxQueue();

    // success return 0,failed return -1 满了会阻塞线程,会被ispushPause变量阻塞
    int push(AVPacket * packet);
    // success return 0,failed return -1,finished return 1 空了会阻塞线程
    int pop(AVPacket * packet);

    int size();

    // 激活死锁的线程，以便进行释放资源
    void activeAllThread();

    void seek(uint64_t seconds, AVFormatContext *formatContext,
                  void (*pFunction)(uint64_t, AVFormatContext *));


};


#endif //MYFFMPEGPLAYER_WLXQUEUE_H
