//
// Created by qq798 on 2018/8/8.
//

#include "WlxQueue.h"


WlxQueue::WlxQueue(WlxPlayerState *playerState) {
    this->playerState = playerState;
    pthread_mutex_init(&queueMutex,NULL);
    pthread_cond_init(&queueEmptyCond,NULL);
    pthread_cond_init(&queueFullCond,NULL);
    pthread_cond_init(&queuePushPauseCond,NULL);

}

WlxQueue::~WlxQueue() {
    // 清空队列
    pthread_cond_signal(&queueEmptyCond);
    pthread_cond_signal(&queueFullCond);
    pthread_cond_signal(&queuePushPauseCond);
    pthread_mutex_lock(&queueMutex);

    while(!queue.empty()){
        AVPacket * packet = queue.front();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
        queue.pop();
    }
    pthread_mutex_unlock(&queueMutex);

    this->playerState = NULL;
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueEmptyCond);
    pthread_cond_destroy(&queueFullCond);
    pthread_cond_destroy(&queuePushPauseCond);
}

int WlxQueue::push(AVPacket *packet) {
    pthread_mutex_lock(&queueMutex);

    while(playerState!=NULL && playerState->currentPlayState!=EnumCurrentPlayState::EXIT){
        if(ispushPause==true){
            pthread_cond_wait(&queuePushPauseCond,&queueMutex);
            if(ISDEBUG){
                LOGD("线程被queuePushPauseCond阻塞了");
            }
        }

        if(queue.size()<queueMaxSize){
            queue.push(packet);

            // 发出线程消息，队列里面有数据了
            pthread_cond_signal(&queueEmptyCond);

            pthread_mutex_unlock(&queueMutex);
            return 0;
        }else{
            // 发出线程消息，防止线程死锁
            if(ISDEBUG){
                LOGD("WlxQueue::push :: 队列满了，线程阻塞");
            }

            pthread_cond_signal(&queueEmptyCond);
            pthread_cond_wait(&queueFullCond,&queueMutex);
        }
    }

    pthread_mutex_unlock(&queueMutex);
    return 0;
}

int WlxQueue::pop(AVPacket *packet) {
    if(packet == NULL){
        if(ISDEBUG){
            LOGE("待赋值的packet没有开辟内存，请先使用av_packet_alloc()赋值！");
        }
        return -1;
    }

    pthread_mutex_lock(&queueMutex);

    while (playerState != NULL && playerState->currentPlayState!=EnumCurrentPlayState::EXIT) {
        if(queue.size()>0){
            AVPacket * avPacket = queue.front();
            if(avPacket!= NULL && av_packet_ref(packet,avPacket)==0){
                // 给返回值赋值成功
                queue.pop();

                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                // 发出线程消息，可以往队列里添数据了
                if(queue.size()< (queueMaxSize/2)){
                    pthread_cond_signal(&queueFullCond);

                    if(ISDEBUG){
                        LOGD("WlxQueue::pop :: 队列少了一半，疏通push线程");
                    }
                }

                pthread_mutex_unlock(&queueMutex);
                return 0;
            }else{
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;

                pthread_mutex_unlock(&queueMutex);
                return -1;
            }

        } else{
            pthread_cond_wait(&queueEmptyCond,&queueMutex);
        }
    }

    pthread_mutex_unlock(&queueMutex);
    return 1;
}

int WlxQueue::size() {
    int size = 0;
    pthread_mutex_lock(&queueMutex);
    size = queue.size();
    pthread_mutex_unlock(&queueMutex);
    return size;
}

void WlxQueue::activeAllThread() {
    pthread_cond_signal(&queueFullCond);
    pthread_cond_signal(&queueEmptyCond);
}

void WlxQueue::seek(uint64_t seconds, AVFormatContext *formatContext,
                    void (*pFunction)(uint64_t, AVFormatContext *)) {
    //阻塞push线程
    ispushPause = true;

    pthread_cond_signal(&queueFullCond);
    pthread_cond_signal(&queueEmptyCond);

    // 锁定队列资源
    pthread_mutex_lock(&queueMutex);
    // 清空队列
    while(!queue.empty()){
        AVPacket * packet = queue.front();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
        queue.pop();
    }
    // 执行回调
    pFunction(seconds,formatContext);
    // 解锁队列资源
    pthread_mutex_unlock(&queueMutex);
    // 解阻塞push线程
    ispushPause = false;
    pthread_cond_signal(&queuePushPauseCond);
}
