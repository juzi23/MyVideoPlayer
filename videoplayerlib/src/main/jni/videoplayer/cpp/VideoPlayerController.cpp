//
// Created by qq798 on 2018/8/28.
//

#include "VideoPlayerController.h"


VideoPlayerController::VideoPlayerController(OnCallJava *pJava) {
    this->onCallJava = pJava;

    synchronizer = new AVSynchronizer(*this, onCallJava);
    audioOutput = new AudioOutput(*this);
    videoOutput = new VideoOutput(*this);

}

VideoPlayerController::~VideoPlayerController() {
    if(synchronizer!= nullptr){
        delete(synchronizer);
        synchronizer = nullptr;
    }

    this->onCallJava = nullptr;
}

void VideoPlayerController::setSource(const char *string) {
    if(synchronizer!= nullptr) {
        synchronizer->setSource(string);
    }
}

void VideoPlayerController::prepare() {
    if(synchronizer!= nullptr){
        synchronizer->prepare();
    }
}

void VideoPlayerController::play() {
    if(synchronizer!= nullptr){
        synchronizer->run();
    }
    if(audioOutput!= nullptr){
        if(audioOutput->isPrepared == false){
            audioOutput->prepare();
        }
        audioOutput->play();
    }

}

void VideoPlayerController::initVideoOutput(ANativeWindow * nativeWindow) {
    // 如果有本地窗口传入，才初始化视频输出部分
    if(videoOutput != nullptr && nativeWindow != nullptr){
        videoOutput->asyncPlay(nativeWindow);
    }
}


