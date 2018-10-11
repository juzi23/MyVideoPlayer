#ifndef VIDEO_PLAYER_MOVIE_FRAME_H
#define VIDEO_PLAYER_MOVIE_FRAME_H

#include <jni.h>
#include <string.h>
#include <malloc.h>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
};


/** 为了避免类文件过多，现在把有关数据结构的model都写到了这个文件中 **/
typedef enum {
    MovieFrameTypeNone,
    MovieFrameTypeAudio,
    MovieFrameTypeVideo
} MovieFrameType;

class MovieFrame{
public:
	// 当前帧时间戳 单位 毫秒
	int64_t position;

	MovieFrame();
	virtual MovieFrameType getType() = 0;
};
class AudioFrame: public MovieFrame{
public:
	// 当前帧时长 单位 毫秒
	float duration;

	// 音频裸数据
	unsigned char * samples;
	// 字节大小
	int size;
	AudioFrame();
	~AudioFrame();
	MovieFrameType getType(){
		return MovieFrameTypeAudio;
	};
	bool dataUseUp;
	void fillFullData();
	void useUpData();
	bool isDataUseUp();
};

class VideoFrame: public MovieFrame{
public:
	// 原始压缩数据
	AVPacket * videoPacket = nullptr;

	// y 裸数据
	uint8_t * luma = nullptr;
	// u 裸数据
	uint8_t * chromaB = nullptr;
	// v 裸数据
	uint8_t * chromaR = nullptr;

	AVFrame * videoAVFrame = nullptr;

	int width;
	int height;
	VideoFrame();
	~VideoFrame();

    // 将当前帧复制
	VideoFrame* clone(){
		VideoFrame* frame = new VideoFrame();
		frame->width = width;
		frame->height = height;
		int lumaLength = width * height;
		frame->luma = new uint8_t[lumaLength];
		memcpy(frame->luma, luma, lumaLength);
		int chromaBLength = width * height / 4;
		frame->chromaB = new uint8_t[chromaBLength];
		memcpy(frame->chromaB, chromaB, chromaBLength);
		int chromaRLength = chromaBLength;
		frame->chromaR = new uint8_t[chromaRLength];
		memcpy(frame->chromaR, chromaR, chromaRLength);
		return frame;
	}

	MovieFrameType getType(){
		return MovieFrameTypeVideo;
	};
};

#endif //VIDEO_PLAYER_MOVIE_FRAME_H

