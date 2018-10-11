//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_VIDEOPLAYERCONTROLLER_H
#define MYVIDEOPLAYER_VIDEOPLAYERCONTROLLER_H

#include "AVSynchronizer.h"
class AVSynchronizer;

#include "AudioOutput.h"
class AudioOutput;
#include "VideoOutput.h"
class VideoOutput;

#include "OnCallJava.h"
#include "WlxPlayerState.h"


class VideoPlayerController {
private:
    AudioOutput * audioOutput = nullptr;
    VideoOutput * videoOutput = nullptr;
    // 当前播放状态，我放在栈区
    WlxPlayerState currentPlayState;
public:
    AVSynchronizer * synchronizer = nullptr;
    OnCallJava * onCallJava = nullptr;
public:
    VideoPlayerController(OnCallJava *pJava);
    ~VideoPlayerController();

    void setSource(const char *string);

    void prepare();

    // 开始播放
    void play();

    void initVideoOutput(ANativeWindow *pWindow);

    // 停止播放
    void stop();

    void resume();

    void pause();
};


#endif //MYVIDEOPLAYER_VIDEOPLAYERCONTROLLER_H
