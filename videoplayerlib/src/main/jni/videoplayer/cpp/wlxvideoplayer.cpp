#include <jni.h>
#include <OnCallJava.h>
#include "AndroidLog.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>

extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#include "VideoPlayerController.h"

// static关键字的作用是将这个静态变量的作用域限制在当前文件中，不加的话，作用域就是整个程序，显然是不合时宜的
static VideoPlayerController * controller = NULL;

static OnCallJava * onCallJava = nullptr;

// 当前java虚拟机
static JavaVM *jvm = nullptr;
// 获取当前java虚拟机
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved)
{
    JNIEnv *env;
    jvm = vm;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK)
    {
        return -1;
    }
    return JNI_VERSION_1_6;
}

//查看wlxvideoplayer版本
extern "C"
JNIEXPORT jstring JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1version(JNIEnv *env, jobject instance) {

    return env->NewStringUTF("version: 1.0");
}

//打印支持的所有编解码器
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1logAllCodec(JNIEnv *env, jobject instance) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL)
    {
        switch (c_temp->type)
        {
            case AVMEDIA_TYPE_VIDEO:
                LOGD("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGD("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGD("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
    return ;
}

//设置播放源
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1setSource(JNIEnv *env, jobject instance,
                                                        jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    if(onCallJava == nullptr){
        onCallJava = new OnCallJava(jvm,env,instance);
    }

    if(controller == NULL){
        controller = new VideoPlayerController(onCallJava);
    }
    controller->setSource(url);

    env->ReleaseStringUTFChars(url_, url);
}

//初始化播放环境
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1prepare(JNIEnv *env, jobject instance) {

    if(controller!=NULL){
        controller->prepare();
    }

}

//开始播放
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1play(JNIEnv *env, jobject instance) {

    if(controller!=NULL){
        controller->play();
    }

}

//暂停播放
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO

}

//继续播放
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1resume(JNIEnv *env, jobject instance) {

    // TODO

}

//停止播放
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1stop(JNIEnv *env, jobject instance) {

    // TODO

}

//切换到指定位置播放
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1seek(JNIEnv *env, jobject instance, jint progress) {

    // TODO

}

//获取帧速率
extern "C"
JNIEXPORT jint JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1getSampleRate(JNIEnv *env, jobject instance) {

    // TODO
    return 0;
}

//打开/关闭录制功能
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1setIsRecoding(JNIEnv *env, jobject instance,
                                                            jboolean isRecoding) {

    // TODO

}

// 初始化视频输出部分
extern "C"
JNIEXPORT void JNICALL
Java_com_wlx_videoplayerlib_WlxVideoPlayer_n_1initVideoOutput(JNIEnv *env, jobject instance,
                                                              jobject surface) {

    ANativeWindow * nativeWindow = ANativeWindow_fromSurface(env, surface);

    controller->initVideoOutput(nativeWindow);

}