//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_AUDIOOUTPUT_H
#define MYVIDEOPLAYER_AUDIOOUTPUT_H

#include "movie_frame.h"
#include "VideoPlayerController.h"
class VideoPlayerController;

extern "C"{
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

class AudioOutput {
public:
    VideoPlayerController *controller = nullptr;
    AudioOutput(VideoPlayerController &);
    ~AudioOutput();

    bool isPrepared = false;
    void prepare();

    void play();

    void pause();

    void resume();

    void stop();

private:
    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

public:
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    AudioFrame * lastAudioFrame = nullptr;

private:
    // 初始化OpenSLES环境
    void initOpenSLES();
    void destoryOpenSLES();

    // 获取音频采样率（枚举 --> 真实的采样率）
    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

};


#endif //MYVIDEOPLAYER_AUDIOOUTPUT_H
