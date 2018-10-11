//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_AUDIOFRAMEQUEUE_H
#define MYVIDEOPLAYER_AUDIOFRAMEQUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include "movie_frame.h"

extern "C"{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
};

#include "AndroidLog.h"

#include "AVSynchronizer.h"
class AVSynchronizer;


class AudioFrameQueue {
private:
    std::queue<AudioFrame *> * queue = nullptr;

    std::mutex mMutex;
    std::condition_variable mCond;
public:
    AudioFrameQueue(AVSynchronizer * synchronizer);
    ~AudioFrameQueue();

public:
    // 当前缓存的总时长，单位 毫秒
    float currentDuration = 0;

    // 上一帧的位置 秒
    int64_t lastFramePositionSecond = 0;
    // 上一帧的位置 毫秒
    int64_t lastFramePositionMSecond = 0;


public:

    // 将新的一帧加入队尾
    void put(AudioFrame *& frame);

    // 成功返回队首元素，失败返回nullptr
    AudioFrame * get();

    AVSynchronizer * synchronizer = nullptr;
};


#endif //MYVIDEOPLAYER_AUDIOFRAMEQUEUE_H
