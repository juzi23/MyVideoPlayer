//
// Created by qq798 on 2018/8/28.
//

#ifndef MYVIDEOPLAYER_VIDEOOUTPUT_H
#define MYVIDEOPLAYER_VIDEOOUTPUT_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "GLUtil.h"
#include "VideoPlayerController.h"
class VideoPlayerController;
class VideoFrame;

class VideoOutput {
public:
    VideoPlayerController * controller = nullptr;
    VideoOutput(VideoPlayerController &);
    ~VideoOutput();

    bool isPrepared = false;
    void prepare();

    std::thread * playThread;
    // 开启线程，进行播放
    void asyncPlay(ANativeWindow * nativeWindow);

    // 渲染视频帧(渲染完毕将销毁videoFrame)
    void renderFrame(VideoFrame * videoFrame);


    void pause();

    void resume();

private:
    // GLSL程序:顶点着色器
    const char * vertexShaderCode =
            "attribute vec4 av_Position;\n"
            "attribute vec2 af_Position;\n"
            "varying vec2 v_texPosition;\n"
            "void main() {\n"
            "    v_texPosition = af_Position;\n"
            "    gl_Position = av_Position;\n"
            "}\n";
    // GLSL程序:片元着色器
    const char * fragmentShaderCode =
            "precision mediump float;\n"
            "varying vec2 v_texPosition;\n"
            "uniform sampler2D sampler_y;\n"
            "uniform sampler2D sampler_u;\n"
            "uniform sampler2D sampler_v;\n"
            "void main() {\n"
            "    float y,u,v;\n"
            "    y = texture2D(sampler_y,v_texPosition).r;\n"
            "    u = texture2D(sampler_u,v_texPosition).r- 0.5;\n"
            "    v = texture2D(sampler_v,v_texPosition).r- 0.5;\n"
            "    vec3 rgb;\n"
            "    rgb.r = y + 1.403 * v;\n"
            "    rgb.g = y - 0.344 * u - 0.714 * v;\n"
            "    rgb.b = y + 1.770 * u;\n"
            "    gl_FragColor = vec4(rgb,1);\n"
            "}\n";

    // 本地窗口
    ANativeWindow * mWindow = nullptr;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    int mWidth;
    int mHeight;

    bool isPlaying = true;

    // 初始化EGL环境
    void initEGL();

    // GLSL程序
    int glslProgram;
    // GLSL程序中的变量 Handle
    GLint avPositionHandle;
    GLint afPositionHandle;
    GLint samplerYHandle;
    GLint samplerUHandle;
    GLint samplerVHandle;
    // GPU中生成的纹理ID数组
    GLuint textureId_yuv[3];

    float * positionArr;
    float * textureCoorArr;

    VideoFrame * lastFrame = nullptr;

    void nativeSurfaceCreated();

    void nativeSurfaceChanged(int width, int height);

    void play();


};


#endif //MYVIDEOPLAYER_VIDEOOUTPUT_H
