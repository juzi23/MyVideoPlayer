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
    stop();

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
    if(currentPlayState.currentPlayState != EnumCurrentPlayState::PLAYING){
        if(synchronizer!= nullptr){
            synchronizer->run();
        }
        if(audioOutput!= nullptr){
            audioOutput->play();
        }
        currentPlayState.currentPlayState = EnumCurrentPlayState::PLAYING;
    }
}

void VideoPlayerController::initVideoOutput(ANativeWindow * nativeWindow) {
    // 如果有本地窗口传入，才初始化视频输出部分
    if(videoOutput != nullptr && nativeWindow != nullptr){
        videoOutput->asyncPlay(nativeWindow);
    }
}

void VideoPlayerController::stop() {
    // 当前c部分有3条线程：
    // 1.ffmpeg拆包线程；2.音频播放线程；3.视频播放解码线程

    // 1.停下3个线程；
    pause();
    // 2.回收资源
    if(videoOutput!= nullptr){
        delete(videoOutput);
        videoOutput = nullptr;
    }
    if(audioOutput!= nullptr){
        delete(audioOutput);
        audioOutput = nullptr;
    }
    if(synchronizer!= nullptr){
        delete(synchronizer);
        synchronizer = nullptr;
    }

    currentPlayState.currentPlayState = EnumCurrentPlayState::EXIT;
}

void VideoPlayerController::resume() {
    if (currentPlayState.currentPlayState == EnumCurrentPlayState::PAUSE) {
        if(synchronizer!= nullptr){
            synchronizer->resume();
        }
        if(audioOutput!= nullptr){
            audioOutput->resume();
        }
        if(videoOutput!= nullptr){
            videoOutput->resume();
        }
        currentPlayState.currentPlayState = EnumCurrentPlayState::PLAYING;
    }
}

void VideoPlayerController::pause() {
    if (currentPlayState.currentPlayState == EnumCurrentPlayState::PLAYING) {
        if(synchronizer!= nullptr){
            synchronizer->pause();
        }
        if(audioOutput!= nullptr){
            audioOutput->pause();
        }
        if(videoOutput!= nullptr){
            videoOutput->pause();
        }
        currentPlayState.currentPlayState = EnumCurrentPlayState::PAUSE;
    }
}


