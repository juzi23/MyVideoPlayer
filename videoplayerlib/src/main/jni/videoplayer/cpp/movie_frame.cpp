
#include "movie_frame.h"

MovieFrame::MovieFrame() {
	position = 0;
}

AudioFrame::AudioFrame() {
	dataUseUp = true;
	samples = NULL;
	size = 0;
    duration = 0.0f;
}
void AudioFrame::fillFullData() {
	dataUseUp = false;
}
void AudioFrame::useUpData() {
	dataUseUp = true;
}
bool AudioFrame::isDataUseUp() {
	return dataUseUp;
}
AudioFrame::~AudioFrame() {
	if (nullptr != samples) {
		free(samples);
		samples = nullptr;
	}
}

VideoFrame::VideoFrame() {
	luma = nullptr;
	chromaB = nullptr;
	chromaR = nullptr;
	videoAVFrame = nullptr;
	width = 0;
	height = 0;
}

VideoFrame::~VideoFrame() {
	luma = nullptr;
	chromaB = nullptr;
	chromaR = nullptr;
	if(videoAVFrame!= nullptr){
		av_frame_free(&videoAVFrame);
		av_free(videoAVFrame);
		videoAVFrame = nullptr;
	}

	if(videoPacket != nullptr){
		av_packet_free(&videoPacket);
		av_free(videoPacket);
		videoPacket = nullptr;
	}
}
