//
// Created by qq798 on 2018/8/28.
//

#include "VideoOutput.h"

VideoOutput::VideoOutput(VideoPlayerController & controller) {
    this->controller = &controller;
}

VideoOutput::~VideoOutput() {
    stop();

    if(playThread->joinable()){
        playThread->join();
    }
    delete(playThread);

    if(positionArr!= nullptr){
        delete[] positionArr;
        positionArr= nullptr;
    }
    if(textureCoorArr != nullptr){
        delete[] textureCoorArr;
        textureCoorArr = nullptr;
    }


    this->controller = nullptr;
}

void VideoOutput::prepare() {
    // 初始化 EGL环境
    initEGL();

    nativeSurfaceCreated();

    nativeSurfaceChanged(mWidth, mHeight);

    isPrepared = true;
}


void VideoOutput::initEGL() {
    // 获取设备屏幕
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY){
        LOGE("display get error!");
        return;
    }
    if(!eglInitialize(display,0,0)){
        LOGE("display init error!");
        return;
    }

    // 配置display
    const EGLint attribs[] {
            EGL_BUFFER_SIZE,32,
            EGL_ALPHA_SIZE,8,
            EGL_BLUE_SIZE,8,
            EGL_GREEN_SIZE,8,
            EGL_RED_SIZE,8,
            EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
            EGL_NONE
    };
    EGLConfig config;
    EGLint configNum;
    if(!eglChooseConfig(display, attribs, &config,1,&configNum)){
        LOGE("eglChooseConfig error!");
        return;
    }

    // 创建OpenGL上下文
    EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
    EGLContext context = eglCreateContext(display, config, NULL, attributes);

    // 获取surface
    EGLSurface surface = NULL;
    EGLint format;
    if(!eglGetConfigAttrib(display,config,EGL_NATIVE_VISUAL_ID,&format)){
        return;
    }
    ANativeWindow_setBuffersGeometry(mWindow,0,0,format);
    surface = eglCreateWindowSurface(display,config,mWindow,0);
    if(!surface){
        return;
    }

    // 当前线程绑定显示资源
    eglMakeCurrent(display,surface,surface,context);


    // 设置宽高
    int width;
    int height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    // 保存初始化后的结果
    mDisplay = display;
    mSurface = surface;
    mContext = context;
    mWidth = width;
    mHeight = height;
}

void VideoOutput::asyncPlay(ANativeWindow * nativeWindow) {
    this->mWindow = nativeWindow;

    playThread = new std::thread(&VideoOutput::play,this);
}

void VideoOutput::renderFrame(VideoFrame * videoFrame) {
    if(videoFrame == nullptr){
//        eglSwapBuffers(mDisplay, mSurface);
        return;
    }

    // 1. 清屏
    glClear(GL_COLOR_BUFFER_BIT);

    // 2.拿出里面的 y u v 数据给渲染器的纹理赋值
    if(videoFrame->width > 0 && videoFrame->height > 0 && videoFrame->luma != nullptr && videoFrame->chromaB != nullptr && videoFrame->chromaR != nullptr)
    {
        glUseProgram(glslProgram);
        // 批量赋值顶点位置坐标
        glEnableVertexAttribArray(avPositionHandle);
        glVertexAttribPointer(avPositionHandle, 2, GL_FLOAT, false, 8, positionArr);
        // 批量赋值顶点纹理坐标
        glEnableVertexAttribArray(afPositionHandle);
        glVertexAttribPointer(afPositionHandle, 2, GL_FLOAT, false, 8, textureCoorArr);
        // 传入y纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId_yuv[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width, videoFrame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->luma);
        // 传入u纹理
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId_yuv[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width / 2, videoFrame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->chromaB);
        // 传入v纹理
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureId_yuv[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width / 2, videoFrame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->chromaR);
        // 用程序中的变量句柄与纹理位置，将纹理与纹理变量真正关联起来
        glUniform1i(samplerYHandle, 0);
        glUniform1i(samplerUHandle, 1);
        glUniform1i(samplerVHandle, 2);

        // 3. 进行渲染,以三角形带的方式画3个点
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 4.缓冲区交换
    eglSwapBuffers(mDisplay, mSurface);

}

void VideoOutput::nativeSurfaceCreated() {
    // 1.加载GLSL程序，获取程序中变量 的 Handle
    glslProgram = GLUtil::createProgram(vertexShaderCode, fragmentShaderCode);
    avPositionHandle = glGetAttribLocation(glslProgram, "av_Position");
    afPositionHandle = glGetAttribLocation(glslProgram, "af_Position");
    samplerYHandle = glGetUniformLocation(glslProgram, "sampler_y");
    samplerUHandle = glGetUniformLocation(glslProgram, "sampler_u");
    samplerVHandle = glGetUniformLocation(glslProgram, "sampler_v");

    // 2.准备用于绘制的长方形：顶点位置属性、顶点纹理坐标属性
    positionArr = new float[8]{
            -1,-1,
            1,-1,
            -1,1,
            1,1
    };
    textureCoorArr= new float[8]{
            0,1,
            1,1,
            0,0,
            1,0
    };

    // 3.在GPU中创建三组纹理
    glGenTextures(3, textureId_yuv);
    // 对三组纹理的环绕方式、缩放模式进行设置
    for(int i = 0; i < 3; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textureId_yuv[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // 缩小模式下为最邻近采样
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // 放大模式下为线性插值
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // 4.设置清屏色--黑色
    glClearColor(0,0,0,1);
}

void VideoOutput::nativeSurfaceChanged(int width, int height) {
    // 设置视口
    glViewport(0,0,width,height);
}

void VideoOutput::play() {
    if(isPrepared == false){
        prepare();
    }

    // 加入播放控制逻辑,如果没有退出或stop，进行暂停与否判断，没暂停，就从同步模块获取一帧进行渲染
    while(true){
        if (!isExit) {
            if (isPlaying) {
                VideoFrame * frame = controller->synchronizer->getCurrentFrame();
                renderFrame(frame);
                // 这里解释下，我在这一步直接释放正在渲染帧时，会导致出现异常，
                // 原因未知(我估计是内存被opengl占用着)，所以我改成释放上一帧
                // 即已经确定渲染完成的帧，这就没问题了
                if(lastFrame != nullptr){
                    delete(lastFrame);
                    lastFrame = nullptr;
                }
                lastFrame = frame;
            }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::this_thread::yield();
            }
        } else{
            // 清屏
            glClear(GL_COLOR_BUFFER_BIT);
            eglSwapBuffers(mDisplay, mSurface);

            // 销毁egl环境
            eglDestroySurface(mDisplay,mSurface);
            eglDestroyContext(mDisplay,mContext);
            break;
        }
    }
}

void VideoOutput::pause() {
    isPlaying = false;
}

void VideoOutput::resume() {
    isPlaying = true;
}

void VideoOutput::stop() {
    isExit = true;
    // 销毁播放线程
    if(playThread != nullptr)
        playThread->join();


}
