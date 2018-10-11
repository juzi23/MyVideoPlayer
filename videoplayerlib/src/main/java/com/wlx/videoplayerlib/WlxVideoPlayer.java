package com.wlx.videoplayerlib;

import android.os.AsyncTask;
import android.view.Surface;

import com.wlx.videoplayerlib.listener.OnFinishedListener;
import com.wlx.videoplayerlib.listener.OnPreparedListener;

import java.io.File;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class WlxVideoPlayer {
    static {
        System.loadLibrary("wlxvideoplayer");
    }

    private ThreadPoolExecutor threadPool;
    public WlxVideoPlayer(){
        threadPool = new ThreadPoolExecutor(3,3,60,TimeUnit.SECONDS,new LinkedBlockingQueue());
    }

    //region 包装了一系列方法 供上层调用...............................................................
    /**
     * 获取版本
     */
    public String getVersion(){
        return n_version();
    }

    /**
     * 打印所有支持的编解码器
     */
    public void logAllCodec(){
        n_logAllCodec();
    }

    /**
     * 初始化播放环境，当播放环境搭建完成，将回调onPreparedListener.OnPrepared()方法
     */
    public void prepare(){
        threadPool.execute(new Runnable() {
            @Override
            public void run() {
                n_prepare();
            }
        });
    }

    /**
     * 停止播放
     */
    public void stop(){
        threadPool.execute(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        });
    }

    /**
     * 设置数据源
     * @param url
     */
    public void setSource(String url){
        n_setSource(url);
    }

    /**
     * 初始化视频输出部分
     * @param surface
     */
    public void initVideoOutput(final Surface surface) {
        threadPool.execute(new Runnable() {
            @Override
            public void run() {
                n_initVideoOutput(surface);
            }
        });
    }

    /**
     * 开始播放
     */
    public void play() {
        n_play();
    }

    public void pause() {
        n_pause();
    }

    public void resume() {
        n_resume();
    }

    public void seek(int progress) {
        n_seek(progress);
    }

    private boolean isMediaCodecInited = false;
    /**
     * 开始录制
     */
    public void startRecord(File file) {

    }

    /**
     * 结束录制
     */
    public void stopRecord() {

    }

    /**
     * 停止录制
     */
    public void pauseRecord(){
        n_setIsRecoding(false);
    }

    /**
     * 继续录制
     */
    public void resumeRecord(){
        n_setIsRecoding(true);
    }

    //endregion

    //region java 调用 c 本地方法....................................................................

    /**
     * 查看ffmpeg版本
     * @return
     */
    private native String n_version();

    /**
     * 打印支持的所有编解码器
     */
    private native void n_logAllCodec();

    /**
     * 设置播放源
     * @param url
     */
    private native void n_setSource(String url);

    private native void n_initVideoOutput(Surface surface);

    /**
     * 初始化播放环境
     */
    private native void n_prepare();

    /**
     * 开始播放
     */
    private native void n_play();

    /**
     * 暂停播放
     */
    private native void n_pause();

    /**
     * 继续播放
     */
    private native void n_resume();

    /**
     * 停止播放
     */
    private native void n_stop();

    /**
     * 切换到指定位置播放
     * @param progress 百分比
     */
    private native void n_seek(int progress);

    /**
     * 获取帧速率
     * @return
     */
    private native int n_getSampleRate();

    /**
     * 打开/关闭录制功能
     * @param isRecoding
     */
    private native void n_setIsRecoding(boolean isRecoding);



    //endregion

    //region 接口回调区

    private OnPreparedListener onPreparedListener;

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void onCallPrepared(){
        if(this.onPreparedListener != null){
            this.onPreparedListener.OnPrepared();
        }
    }



    //endregion
}
