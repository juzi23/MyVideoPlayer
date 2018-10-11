#include "VideoFrameQueue.h"

VideoFrameQueue::VideoFrameQueue() {
    queue = new std::queue<VideoFrame *>();
}

VideoFrameQueue::~VideoFrameQueue() {
    delete(queue);
}

void VideoFrameQueue::put(VideoFrame *& frame) {

    std::unique_lock<std::mutex> lck(mMutex);

    // 遍历，查找到刚好position大于frame的position的迭代器位置，插入到它的前面
    bool isInserted = false;
//    if(queue->size()>1) {
//        for (auto itr = queue->begin(); itr != queue->end(); itr++) {
//            if ((*itr)->position > frame->position) {
//                queue->insert(itr, frame);
//                isInserted = true;
//                break;
//            }
//        }
//    }

    // 否则的话，插入队列末尾
    if(isInserted == false){
        queue->push(frame);
    }

    mCond.notify_all();
    lck.unlock();


}

VideoFrame *VideoFrameQueue::get() {
    VideoFrame * frame = nullptr;

    std::unique_lock<std::mutex> lck(mMutex);
    // 校验长度
    if(queue->size() <= 0){
        // 应该进行线程等待
        mCond.wait(lck);
    }

    // 获取队首元素
    frame = static_cast<VideoFrame *>(queue->front());

    queue->pop();


    lck.unlock();
    if(frame != nullptr)
        LOGD("当前获取到的视频帧position: %lld",frame->position);
    return frame;
}
