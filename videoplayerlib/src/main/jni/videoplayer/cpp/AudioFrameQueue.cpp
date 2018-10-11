//
// Created by qq798 on 2018/8/28.
//

#include "AudioFrameQueue.h"

AudioFrameQueue::AudioFrameQueue(AVSynchronizer * synchronizer) {
    this->synchronizer = synchronizer;
    queue = new std::queue<AudioFrame *>();

}

AudioFrameQueue::~AudioFrameQueue() {
    delete(queue);
}



void AudioFrameQueue::put(AudioFrame *& frame) {
    // 校验帧时间戳、帧时长
    if(frame->duration <= 0){
        if(queue->size() <=0 ){
            frame->duration = 0;
            frame->position = 0;
        } else{
            // 如果时长为空，那么这帧的持续时长表示为 当前帧时间戳 - 上一帧时间戳
            frame->duration = (frame->position - queue->back()->position);
        }
    }

    std::unique_lock<std::mutex> lck(mMutex);
    queue->push(frame);
    // 更新缓存时长
    currentDuration += frame->duration;
    mCond.notify_all();
    lck.unlock();
}

AudioFrame *AudioFrameQueue::get() {
    AudioFrame * frame = nullptr;

    std::unique_lock<std::mutex> lck(mMutex);

    // 校验长度
    if(queue->size() <= 0){
        // 应该进行线程等待
        mCond.wait(lck);
    }

    // 获取队首元素
    frame = queue->front();

    // 移除队首元素
    queue->pop();
    // 更新缓存时长
    currentDuration -= frame->duration;

    lck.unlock();

    LOGD("当前获取到的音频帧position: %lld",frame->position);
    return frame;
}
