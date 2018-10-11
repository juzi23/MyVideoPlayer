//
// Created by qq798 on 2018/8/7.
//

#ifndef MYFFMPEGPLAYER_ONCALLJAVA_H
#define MYFFMPEGPLAYER_ONCALLJAVA_H

#include <jni.h>
#include "AndroidLog.h"

#define MAINTHREAD 0
#define CHILDTHREAD 1

class OnCallJava{
public :
    JavaVM * jvm = NULL;
    JNIEnv * jenv = NULL;
    jobject jobj = NULL;
    jclass jclz = NULL;

private:
    jmethodID jmid_onCallPrepared;
public :
    OnCallJava(JavaVM *,JNIEnv *,jobject);
    ~OnCallJava();

    // 准备完成的回调，在子线程中调用
    void callPrepared();
};

#endif //MYFFMPEGPLAYER_ONCALLJAVA_H
