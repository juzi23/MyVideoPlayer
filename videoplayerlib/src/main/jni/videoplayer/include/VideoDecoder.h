//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_VIDEODECODER_H
#define MYVIDEOPLAYER_VIDEODECODER_H

#include <memory>

#include <thread>
#include <mutex>
#include <condition_variable>
#include "AndroidLog.h"
#include "OnCallJava.h"
#include "AVSynchronizer.h"
class AVSynchronizer;

#include <movie_frame.h>
class MovieFrame;
class AudioFrame;
class VideoFrame;

extern "C"{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

class AVSynchronizer;

class VideoDecoder {
// region 音频解码部分字段
public:
    // 音频流在源文件上下文中的序号
    int audioIndex;

    // 音频流时长单位 秒
    int64_t audioDuration;
    // 音频解码器
    AVCodec * audioCodec = nullptr;
    // 解码器上下文
    AVCodecContext * audioCodecContext = nullptr;

    AVRational audioTimeBase;

    // 音频重采样上下文
    SwrContext *audioSwrContext = nullptr;

    void pause();

    void resume();

public:
    // 音频解码器参数
    AVCodecParameters * audioCodecpar = nullptr;
//endregion

//region 视频解码部分字段
public:
    int videoIndex = 0;

    AVCodecParameters * videoCodecpar = nullptr;

    // 视频流的总时长 单位 秒
    int64_t videoDuration = 0;

    AVRational videoTimebase;

    // 视频解码器
    AVCodec * videoCodec = nullptr;

    //视频解码器上下文
    AVCodecContext * videoCodecContext = nullptr;

    // 视频重采样的缓冲区
    uint8_t *buffer = nullptr;

    // 视频重采样上下文
    SwsContext * videoSwrContext = nullptr;

//endregion

public:
    AVSynchronizer *synchronizer;

    char * url = nullptr;
    // 源文件格式上下文
    AVFormatContext *formatContext;

    // 准备线程的退出标志位
    bool isPrepareThreadExit;
    // 进行准备工作的线程
    std::thread * prepareThread;
    std::mutex prepareMutex;
    std::condition_variable prepareCond;

    // 进行解码工作的线程
    std::thread * decodeThread;
    std::mutex decodeMutex;
    std::condition_variable decodeCond;
    // 是否进行解码工作的标志位
    bool isOnDecoding = false;

    // 发送进行解码的消息
    void requestDecode();

    // 是否仅包含音频的标志位
    bool isAudioOnly = true;

public:
    OnCallJava *onCallJava = nullptr;
public:
    VideoDecoder(AVSynchronizer &, OnCallJava *&);
    ~VideoDecoder();

    void setSource(const char *string);

    // 进行异步的prepare
    void asyncPrepare();
    // 启动解码线程
    void startDecode();

    // 对videoFrame中的原始数据进行解码
    // success return true ,else return false
    bool decodeVideoAVPacket(VideoFrame *&);

private:
    // 在子线程中进行准备
    void prepare();

    // 在子线程中进行解码
    void decode();



    // AVPacket --> AudioFrame s 然后发送到音频队列缓冲中
    void decodeAudioAVPacket(AVPacket *&);

    double synchronize(AVFrame *srcFrame, double pts);

    double video_clock = 0;
};



#endif //MYVIDEOPLAYER_VIDEODECODER_H
