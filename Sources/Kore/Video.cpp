#include "pch.h"
#include "Video.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Texture.h>
#include <Kore/System.h>
#include <TheoraPlayer.h>
#include <Kore/VideoSoundStream.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

VideoSoundStream::VideoSoundStream(TheoraVideoClip* owner, int nChannels, int freq) : TheoraAudioInterface(owner, nChannels, freq), bufferSize(1024 * 100 * 3), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {
	buffer = new float[bufferSize];
}

void VideoSoundStream::insertData(float* data, int nSamples) {
	written += nSamples;
	int remaining = nSamples;
	for (int i = 0; i < nSamples; ++i) {
		buffer[bufferWritePosition++] = data[i];
		if (bufferWritePosition >= bufferSize) {
			bufferWritePosition = 0;
		}
	}
}

float VideoSoundStream::nextSample() {
	++read;
	if (written <= read) return 0;
	if (bufferReadPosition >= bufferSize) bufferReadPosition = 0;
	return buffer[bufferReadPosition++];
}

bool VideoSoundStream::ended() {
	return false;
}

class KoreAudioInterfaceFactory : public TheoraAudioInterfaceFactory {
public:
	TheoraAudioInterface* createInstance(TheoraVideoClip* owner, int nChannels, int freq) {
		VideoSoundStream* stream = new VideoSoundStream(owner, nChannels, freq);
		Mixer::play(stream);
		return stream;
	}
};

namespace {
	TheoraVideoManager* theora = nullptr;
}

const char* iphonegetresourcepath();

Video::Video(const char* filename) {
	if (theora == nullptr) {
		theora = new TheoraVideoManager();
		theora->setAudioInterfaceFactory(new KoreAudioInterfaceFactory());
	}
#ifdef SYS_IOS
	char name[2048];
	strcpy(name, iphonegetresourcepath());
	strcat(name, "/");
	strcat(name, KORE_DEBUGDIR);
	strcat(name, "/");
	strcat(name, filename);
	clip = theora->createVideoClip(name, TH_RGB, 16);
#else
	clip = theora->createVideoClip(filename, TH_RGB, 16);
#endif
	clip->pause();
	image = new Texture(width(), height(), Image::RGBA32, false);
}

Video::~Video() {
	
}

void Video::play() {
	clip->play();
	lastTime = System::time();
}

void Video::pause() {
	clip->pause();
}

void Video::update(double time) {
	
}

int Video::width() {
	return clip->getWidth();
}

int Video::height() {
	return clip->getHeight();
}

Texture* Video::currentImage() {
	double currentTime = System::time();
	theora->update(static_cast<float>(currentTime - lastTime));
	lastTime = currentTime;

	TheoraVideoFrame* f = clip->getNextFrame();
	unsigned char* data = f->getBuffer();
	u8* pixel = image->lock();
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
#ifdef OPENGL
			pixel[y * image->stride() + x * 4 + 0] = data[y * width() * 3 + x * 3 + 0];
			pixel[y * image->stride() + x * 4 + 1] = data[y * width() * 3 + x * 3 + 1];
			pixel[y * image->stride() + x * 4 + 2] = data[y * width() * 3 + x * 3 + 2];
			pixel[y * image->stride() + x * 4 + 3] = 255;
#else
			pixel[y * image->stride() + x * 4 + 0] = data[y * width() * 3 + x * 3 + 2];
			pixel[y * image->stride() + x * 4 + 1] = data[y * width() * 3 + x * 3 + 1];
			pixel[y * image->stride() + x * 4 + 2] = data[y * width() * 3 + x * 3 + 0];
			pixel[y * image->stride() + x * 4 + 3] = 255;
#endif
		}
	}
	image->unlock();
	return image;
}
