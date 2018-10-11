//
// Created by qq798 on 2018/8/28.
//


#include "AVSynchronizer.h"


AVSynchronizer::AVSynchronizer(VideoPlayerController & controller, OnCallJava *& onCallJava) {
    this->controller = &controller;
    this->onCallJava = onCallJava;

    this->videoDecoder = new VideoDecoder(*this, onCallJava);

    this->audioQueue = new AudioFrameQueue(this);
    this->videoQueue = new VideoFrameQueue();

}

AVSynchronizer::~AVSynchronizer() {

    if(videoQueue != nullptr){
        delete(videoQueue);
        videoQueue = nullptr;
    }

    if(audioQueue != nullptr){
        delete(audioQueue);
        audioQueue = nullptr;
    }

    if(videoDecoder != nullptr){
        delete(videoDecoder);
        videoDecoder = nullptr;
    }

    this->controller = nullptr;
    this->onCallJava = nullptr;
}

void AVSynchronizer::setSource(const char *string) {
    if(videoDecoder != nullptr)
        videoDecoder->setSource(string);
}

void AVSynchronizer::prepare() {
    if(videoDecoder!= nullptr){
        videoDecoder->asyncPrepare();
    }
}

AudioFrame * AVSynchronizer::getAudioFrame() {
    //如果音频文件低于最小值时，需要请求填充缓冲区

    if(audioQueue->currentDuration<minDuration){
        videoDecoder->requestDecode();
    }

    AudioFrame * frame = audioQueue->get();

    // 记录下最后一次拿走的音频时间戳
    lastAudioFrameTimeStamp = frame->position;
    // 记录下拿走时的时间(毫秒级)
    lastGetSystemTime = getCurrentTimeMS();

    return frame;
}

VideoFrame *AVSynchronizer::getCurrentFrame() {
    // 获取到当前音频播放时间
    long long tempDuration  =getCurrentTimeMS() - lastGetSystemTime;
    long long currentAudioTime = lastAudioFrameTimeStamp + tempDuration;

    VideoFrame * outFrame = videoQueue->get();

    if(outFrame == nullptr)
        return nullptr;

    // 解码
    if(! videoDecoder->decodeVideoAVPacket(outFrame)){
        delete(outFrame);
        return nullptr;
    }

    if(outFrame->position>currentAudioTime){
        long long minin = (outFrame->position - currentAudioTime);
        std::this_thread::sleep_for(std::chrono::milliseconds(minin/10*9));
        std::this_thread::yield();
    }

    return outFrame;
}

void AVSynchronizer::run() {
    if(videoDecoder != nullptr){
        videoDecoder->startDecode();
    }
}

void AVSynchronizer::pause() {
    // ①停下videoDecoder中的解码线程
    videoDecoder->pause();
}

void AVSynchronizer::resume() {
    videoDecoder->resume();
}


