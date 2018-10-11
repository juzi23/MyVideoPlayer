//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_AVSYNCHRONIZER_H
#define MYVIDEOPLAYER_AVSYNCHRONIZER_H

#include <chrono>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "VideoPlayerController.h"
class VideoPlayerController;

#include "AndroidLog.h"
#include "VideoDecoder.h"
class VideoDecoder;

#include "VideoFrameQueue.h"
class VideoFrameQueue;
#include "AudioFrameQueue.h"
class AudioFrameQueue;

#include <movie_frame.h>
class MovieFrame;
class AudioFrame;
class VideoFrame;

#include "OnCallJava.h"
#include "MyTime.h"

// 承上
class VideoPlayerController;
// 启下
class VideoDecoder;

class AVSynchronizer {
    //构造区
public:
    AVSynchronizer(VideoPlayerController &, OnCallJava *&);

    ~AVSynchronizer();

    //私有属性区
private:

    //公共变量区
public:
    // 解码器
    VideoDecoder * videoDecoder = nullptr;
    VideoPlayerController * controller;
    OnCallJava * onCallJava;

    // 视频帧缓冲队列
    VideoFrameQueue * videoQueue = nullptr;
    // 音频帧缓冲队列
    AudioFrameQueue * audioQueue = nullptr;

    std::mutex syncMutex;
    std::condition_variable syncCond;


    // 私有方法区
private:

    // 公共方法区
public:
    void setSource(const char *string);

    void prepare();

    // 获取音频帧
    AudioFrame * getAudioFrame();

    // 获取正确的视频帧
    VideoFrame * getCurrentFrame();

    VideoFrame * lastFrame = nullptr;
    VideoFrame * nextFrame = nullptr;


    void run();

    // 帧缓冲区的最大时长 单位 毫秒
    const int64_t maxDuration = 800;
    // 帧缓冲区的最小时长 单位 毫秒
    const int64_t minDuration = 200;

    // 最近一次被拿走的音频帧时间戳
    int64_t lastAudioFrameTimeStamp = 0;
    // 记录下拿走时的时间
    long long lastGetSystemTime;
};


#endif //MYVIDEOPLAYER_AVSYNCHRONIZER_H
