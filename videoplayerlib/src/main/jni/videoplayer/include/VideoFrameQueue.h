#ifndef MYVIDEOPLAYER_VIDEOFRAMEQUEUE_H
#define MYVIDEOPLAYER_VIDEOFRAMEQUEUE_H

#include "movie_frame.h"
class VideoFrame;

#include <mutex>
#include <condition_variable>
#include <queue>
#include "AndroidLog.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
};

class VideoFrameQueue {
private:
    std::queue<VideoFrame *> * queue;

    std::mutex mMutex;
    std::condition_variable mCond;
public:
    VideoFrameQueue();
    ~VideoFrameQueue();

    // 将新的videoFrame放入队首
    void put(VideoFrame *&);

    // 取出队首videoFrame
    VideoFrame *get();

};


#endif //MYVIDEOPLAYER_VIDEOFRAMEQUEUE_H
