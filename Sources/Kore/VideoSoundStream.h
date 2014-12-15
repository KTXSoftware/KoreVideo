#pragma once

#include <TheoraAudioInterface.h>

namespace Kore {
	class VideoSoundStream : public TheoraAudioInterface {
	public:
		VideoSoundStream(TheoraVideoClip* owner, int nChannels, int freq);
		void insertData(float* data, int nSamples);
		float nextSample();
		bool ended();
	private:
		float* buffer;
		const int bufferSize;
		int bufferWritePosition;
		int bufferReadPosition;
		u64 read;
		u64 written;
	};
}

#if 0
struct vorbis_dsp_state;
struct vorbis_block;

#include <ogg/ogg.h>

namespace Kore {
	class VideoSoundStream {
	public:
		VideoSoundStream(vorbis_dsp_state* v_state, ogg_stream_state* o_vsstate, ogg_packet* o_packet, vorbis_block* v_block);
		float nextSample();
		bool ended();
		vorbis_dsp_state* v_state;
		ogg_stream_state* o_vsstate;
		ogg_packet* o_packet;
		vorbis_block* v_block;
		float* buffer;
		int bufferSize;
		int bufferReadPosition;
		int bufferWritePosition;
		int read;
		int written;
	};
}
#endif
