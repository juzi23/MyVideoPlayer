#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(AVSynchronizer & sync, OnCallJava *& onCallJava) {
    this->synchronizer = &sync;
    this->onCallJava = onCallJava;
}

VideoDecoder::~VideoDecoder() {
    //TODO 修改控制子线程工作的标志位

    //等待子线程结束
    prepareThread->join();
    decodeThread->join();

    //回收内存
    delete(prepareThread);
    delete(decodeThread);

    this->synchronizer = nullptr;

    if(url != nullptr){
        free(url);
        url = nullptr;
    }
}

void VideoDecoder::setSource(const char *string) {
    // 要拷贝，指针传递会被销毁
    size_t strlen1 = strlen(string);
    url = static_cast<char *>(malloc(strlen1 + 1));
    strcpy(url, string);
}

void VideoDecoder::asyncPrepare() {
    prepareThread = new std::thread(&VideoDecoder::prepare,this);
}


void VideoDecoder::startDecode() {
    if(url == nullptr){
        LOGE("缺少音频文件，无法播放");
        return;
    }

    decodeThread = new std::thread(&VideoDecoder::decode,this);
}

void VideoDecoder::decode() {
    isOnDecoding = true;
    while (true) {
        if (isOnDecoding) {
            LOGD("当前音频缓冲时长%d",(int)(synchronizer->audioQueue->currentDuration));

            // 每解码一帧，都进行判断，如果音频队列中的数据都大于最大刻度时，线程处于等待状态
            // 还有一种情况，如果仅存在音频
            bool firstCond = synchronizer->audioQueue->currentDuration >= synchronizer->maxDuration;

            if (firstCond){
                // 声明解码锁
                std::unique_lock<std::mutex> lckDecode(decodeMutex);
                decodeCond.wait(lckDecode);
            }

            //region 循环体
            int ret = -1;

            // 分配空包
            AVPacket * packet = av_packet_alloc();

            ret = av_read_frame(formatContext,&(*packet));

            if(ret != 0 || packet == nullptr){
                LOGD("读取完毕");
                av_packet_free(&packet);
                av_free(packet);
                packet = nullptr;
                break;
            }
            ret = -1;
            //endregion

            if(packet->stream_index == audioIndex) {
                // 解码音频包
                decodeAudioAVPacket(packet);

                av_packet_free(&packet);
                av_free(packet);
                packet = nullptr;

                continue;
            }
            else if(packet->stream_index == videoIndex){
                // 将packet存储到视频缓冲队列中
                VideoFrame * videoFrame = new VideoFrame();
                videoFrame->videoPacket = packet;

                // 记录当前帧时间戳 转化为毫秒 这是根据packet中的值粗略估计的
                int64_t pts = 0 ;
                pts = (packet->pts * 1000*videoTimebase.num) / videoTimebase.den;
                videoFrame->position = pts;

                // 将videoFrame放入VideoFrameQueue中
                synchronizer->videoQueue->put(videoFrame);

                continue;
            }

            continue;
            //endregion
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::this_thread::yield();
        }
    }

    // TODO 解码完成
}

void VideoDecoder::requestDecode() {
    std::unique_lock<std::mutex> lckDecode(decodeMutex);
    decodeCond.notify_one();
}

void VideoDecoder::prepare() {
    if(url == nullptr){
        LOGE("VideoDecoder::prepare() error : url is null");

        return;
    }
    // 声明一个唯一锁
    std::unique_lock<std::mutex> lck(prepareMutex);
    // 线程准备结束的标志设置为空
    isPrepareThreadExit = false;

    int ret = -1;

    av_register_all();
    avformat_network_init();

    // 分配格式上下文
    formatContext = avformat_alloc_context();

    // 设置连接时回调方法
    formatContext->interrupt_callback.callback = [](void * ctx)->int{
        VideoDecoder * decoder = (VideoDecoder *)ctx;

        // TODO 设置回调超时
        //return AVERROR_EOF;

        return 0;
    };
    formatContext->interrupt_callback.opaque = this;

    // 根据源文件地址，获得源文件格式上下文(请提供网络权限)
    ret = avformat_open_input(&formatContext,url,NULL,NULL);
    if(ret!=0){
        LOGE("不能生成格式上下文，请检查 %s 地址是否有效！",url);

        isPrepareThreadExit = true;
        return;
    }
    ret = -1;

    // 查询流信息，获取音频流
    ret = avformat_find_stream_info(formatContext,NULL);
    if(ret!=0){
        LOGE("该url地址不包含有效的音视频流！");

        isPrepareThreadExit = true;
        return;
    }
    ret = -1;

    // 音频流在源文件上下文中的序号
    int audioIndex = -1;
    // 视频流在源文件上下文中的序号
    int videoIndex = -1;
    for(int i=0;i<formatContext->nb_streams;i++){
        if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
           && audioIndex == -1){
            audioIndex = i;
        } else if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
                  && videoIndex == -1){
            videoIndex = i;
        }
    }
    // 音频流很重要，没有音频流就不播放了
    if (audioIndex == -1) {
        LOGE("该url地址不包含有效的音频流！");

        isPrepareThreadExit = true;
        return;
    }

    // 视频流可有可无
    if(videoIndex == -1){
        LOGE("该url地址不包含有效的视频流！");
    }

    //region 初始化音频部分
    // 在同步模块中，设置音频帧缓冲队列
    if(synchronizer->audioQueue == nullptr)
        synchronizer->audioQueue = new AudioFrameQueue(synchronizer);
    this->audioIndex = audioIndex;
    audioCodecpar = formatContext->streams[audioIndex]->codecpar;
    audioDuration = formatContext->duration / AV_TIME_BASE;
    audioTimeBase = formatContext->streams[audioIndex]->time_base;

    // 获取音频流解码器
    audioCodec = avcodec_find_decoder(audioCodecpar->codec_id);
    if(!audioCodec){
        LOGE("没有找到相应解码器，解码器ID为 %d ！",audioCodecpar->codec_id);

        delete(synchronizer->audioQueue);

        isPrepareThreadExit = true;
        return;
    }

    LOGD("当前音频解码器名字为 : %s",audioCodec->name);

    // 获取解码器上下文(空)
    audioCodecContext = avcodec_alloc_context3(audioCodec);
    if(!audioCodecContext){
        LOGE("解码器上下文创建失败");

        delete(synchronizer->audioQueue);

        isPrepareThreadExit = true;
        return;
    }
    // 将解码参数写入解码器上下文中
    ret = avcodec_parameters_to_context(audioCodecContext,audioCodecpar);
    if(ret!=0){
        LOGE("解码器参数拷贝失败");

        delete(synchronizer->audioQueue);

        isPrepareThreadExit = true;
        return;
    }
    ret = -1;

    // 打开解码器
    ret = avcodec_open2(audioCodecContext,audioCodec,NULL);
    if(ret!=0){
        LOGE("解码器打开失败");

        delete(synchronizer->audioQueue);

        isPrepareThreadExit = true;
        return;
    }
    ret = -1;
    //endregion


    // 如果媒体包含视频流
    //region 初始化视频部分
    if (videoIndex != -1) {
        // 在同步模块中，设置视频帧缓冲队列
        if(synchronizer->videoQueue == nullptr)
            synchronizer->videoQueue = new VideoFrameQueue();
        this->videoIndex = videoIndex;
        videoCodecpar = formatContext->streams[videoIndex]->codecpar;
        videoDuration = formatContext->duration / AV_TIME_BASE;
        videoTimebase = formatContext->streams[videoIndex]->time_base;

        // 获取视频流解码器
        videoCodec = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);

        if(!videoCodec){
            LOGE("没有找到相应解码器，解码器ID为 %d ！",videoCodecpar->codec_id);

            delete(synchronizer->videoQueue);

            isPrepareThreadExit = true;
//            return;
            ret = -2;
            goto videoopenerror;
        }


        LOGD("当前视频解码器名字为 : %s",videoCodec->name);

        // 获取解码器上下文(空)
        videoCodecContext = avcodec_alloc_context3(videoCodec);
        if(!videoCodecContext){
            LOGE("解码器上下文创建失败");

            delete(synchronizer->videoQueue);

            isPrepareThreadExit = true;

            ret = -2;
            goto videoopenerror;
        }

        // 将解码参数写入解码器上下文中
        ret = avcodec_parameters_to_context(videoCodecContext,videoCodecpar);
        if(ret!=0){
            LOGE("解码器参数拷贝失败");

            delete(synchronizer->videoQueue);

            isPrepareThreadExit = true;

            ret = -2;
            goto videoopenerror;
        }
        ret = -1;

        // 打开解码器
        int openTimes = 0;
        while(ret != 0) {
            ret = avcodec_open2(videoCodecContext,videoCodec, NULL);
            if (ret != 0) {
                LOGE("解码器打开失败");

                delete (synchronizer->videoQueue);

                isPrepareThreadExit = true;

                // 有5次打开机会
                if(openTimes >= 5){
                    ret = -2;
                    goto videoopenerror;
                }

                continue;
            }else{
                break;
            }
        }
        ret = -1;
    }
    //endregion

    // 视频部分打开失败
    videoopenerror:
    if(ret == -2 ){
        LOGD("视频部分打开失败");
    }


    onCallJava->callPrepared();

    isPrepareThreadExit = true;
}

bool VideoDecoder::decodeVideoAVPacket(VideoFrame *& videoFrame) {
    AVPacket * srcPacket = videoFrame->videoPacket;
    int ret = -1;
    // 将待解码的包发送到解码器上下文中
    while(1){
        ret = avcodec_send_packet(this->videoCodecContext, srcPacket);
        if (ret != 0) {
            return false;
        }
        ret = -1;

        // 解码，并将解码后数据保存到frame中
        AVFrame * dscFrame = av_frame_alloc();

        ret = avcodec_receive_frame(this->videoCodecContext, &(*dscFrame));
        if(AVERROR(EAGAIN) == ret){
            av_frame_free(&dscFrame);
            av_free(dscFrame);
            continue;
        }
        if(0 == ret){
            // 进行重采样 AVFrame --> VideoFrame

//            VideoFrame * videoFrame = new VideoFrame();
            videoFrame->width = videoCodecContext->width;
            videoFrame->height = videoCodecContext->height;

//            // 记录当前帧时间戳 转化为毫秒
//            int64_t pts = 0 ;
//            if ((pts = av_frame_get_best_effort_timestamp(&(*dscFrame))) == AV_NOPTS_VALUE){
//                pts = srcPacket->pts * 1000 * videoTimebase.num / videoTimebase.den;
//            }else{
//                pts = (pts * 1000*videoTimebase.num) / videoTimebase.den;
//            }
//            videoFrame->position = pts;

            //region 统一输出帧格式 到 videoFrame 中
            if(dscFrame->format == AV_PIX_FMT_YUV420P)
            {
                LOGD("当前视频是YUV420P格式");

                // frame --> videoFrame

                // y分量
                videoFrame->luma = dscFrame->data[0];
                // u分量
                videoFrame->chromaB = dscFrame->data[1];
                // v分量
                videoFrame->chromaR = dscFrame->data[2];
                videoFrame->videoAVFrame = dscFrame;

            } else{
                LOGD("当前视频不是YUV420P格式");

                // 转换完成后的裸数据
                AVFrame *pFrameYUV420P = av_frame_alloc();

                //region 进行重采样
                // 判断初始化重采样缓冲区
                if (buffer == nullptr) {
                    int num = av_image_get_buffer_size(
                            AV_PIX_FMT_YUV420P,
                            videoCodecContext->width,
                            videoCodecContext->height,
                            1);
                    buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                }
                av_image_fill_arrays(
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize,
                        buffer,
                        AV_PIX_FMT_YUV420P,
                        videoCodecContext->width,
                        videoCodecContext->height,
                        1);
                // 判断构建重采样上下文
                if (videoSwrContext == nullptr) {
                    videoSwrContext = sws_getContext(
                            videoCodecContext->width,
                            videoCodecContext->height,
                            videoCodecContext->pix_fmt,
                            videoCodecContext->width,
                            videoCodecContext->height,
                            AV_PIX_FMT_YUV420P,
                            SWS_BICUBIC, NULL, NULL, NULL);
                    // 如果不能生成重采样上下文
                    if(!videoSwrContext)
                    {
                        av_frame_free(&pFrameYUV420P);
                        av_free(pFrameYUV420P);
                        av_free(buffer);
                        return false;
                    }
                }
                // 进行重采样
                sws_scale(
                        videoSwrContext,
                        reinterpret_cast<const uint8_t *const *>(dscFrame->data),
                        dscFrame->linesize,
                        0,
                        dscFrame->height,
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize);
                //endregion

                // pFrameYUV420P --> videoFrame

                // y分量
                videoFrame->luma = pFrameYUV420P->data[0];
                // u分量
                videoFrame->chromaB = pFrameYUV420P->data[1];
                // v分量
                videoFrame->chromaR = pFrameYUV420P->data[2];
                videoFrame->videoAVFrame = pFrameYUV420P;

                av_frame_free(&dscFrame);
                av_free(dscFrame);
            }
            //endregion

            // 继续处理下一帧
            return true;
        }else{
            return false;
        }
    }
    ret = -1;
}

void VideoDecoder::decodeAudioAVPacket(AVPacket *& packet) {
    int ret = -1;

    // 将待解码的包发送到解码器上下文中
    ret = avcodec_send_packet(audioCodecContext, &(*packet));
    if (ret != 0) {
        av_packet_free(&packet);
        av_free(packet);
        packet = nullptr;
        return;
    }
    ret = -1;

    // region 循环取packet中的frame,直到取干净
    while(1){

        // 解码，并将解码后数据保存到frame中
        std::shared_ptr<AVFrame> frame(av_frame_alloc(),[](AVFrame * frame){
            av_frame_free(&frame);
            av_free(frame);
            frame = nullptr;
        });
        ret = avcodec_receive_frame(audioCodecContext, &(*frame));
        // 如果取干净了
        if (ret != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            return;
        }
        ret = -1;
        //endregion

        AudioFrame *audioFrame = new AudioFrame();
        // 分配一秒的缓冲 = 采样率（每秒采样点 * 声道数 * 每个采样点的字节数）
        audioFrame->samples = static_cast<uint8_t *>(malloc(audioCodecpar->sample_rate
                                                            * audioCodecpar->channels
                                                            * av_get_bytes_per_sample((AVSampleFormat) (audioCodecpar->format))));
        // 记录当前帧时间戳 转化为秒
        audioFrame->position = frame->pts *1000 * av_q2d(audioTimeBase);
        // audioFrame->position = frame->pts;
        // 记录当前帧时长 转化为毫秒
        audioFrame->duration = 1000 * frame->pkt_duration * av_q2d(audioTimeBase);

        //region 对音频进行重采样,重采样后的数据存放到AudioFrame中
        // 校正声道数和声道布局
        if (frame->channels > 0 && frame->channel_layout == 0) {
            // 声道布局有问题时
            frame->channel_layout = av_get_default_channel_layout(frame->channels);
        } else if (frame->channel_layout > 0 && frame->channels == 0) {
            // 声道数有问题
            frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);
        }

        // 构建重采样上下文
        if (audioSwrContext == nullptr) {
            audioSwrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    frame->sample_rate,
                    frame->channel_layout,
                    (AVSampleFormat) (frame->format),
                    frame->sample_rate,
                    NULL, NULL
            );
        }
        // 初始化重采样上下文
        if (audioSwrContext == NULL || swr_init(audioSwrContext) < 0) {
            swr_free(&audioSwrContext);

            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            return;
        }

        // 进行重采样
        audioFrame->size = swr_convert(audioSwrContext,
                                       &audioFrame->samples,
                                       frame->nb_samples,
                                       (const uint8_t **) (frame->data),
                                       frame->nb_samples);
        // 得到字节大小
        audioFrame->size *= av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO) *
                            av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        //endregion

        // 送入AudioFrameQueue队列中
        synchronizer->audioQueue->put(audioFrame);

        // 此时frame已经用完了，智能指针自动释放
    }
    // endregion

    // 继续处理下一帧
}

double VideoDecoder::synchronize(AVFrame *srcFrame, double pts)
{
    double frame_delay;

    if (pts != 0)
        video_clock = pts; // Get pts,then set video clock to it
    else
        pts = video_clock; // Don't get pts,set it to video clock

    frame_delay = av_q2d(audioTimeBase);
    frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);

    video_clock += frame_delay;

    return pts;
}

void VideoDecoder::pause() {
    isOnDecoding = false;
}

void VideoDecoder::resume() {
    isOnDecoding = true;
}



