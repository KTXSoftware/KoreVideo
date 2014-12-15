#include "pch.h"
#include "Video.h"
#include "oggplayer.h"
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

#if 0
OggPlayer* player;
Texture* image;
char* videodata;

Video::Video(const char* filename) {
	player = new OggPlayer(filename, AF_S16, 2, 44100);
	videodata = new char[width() * height() * 4];
	image = new Texture(width(), height(), Image::RGBA32, false);
}

Video::~Video() {
	delete player;
	player = nullptr;
}

void Video::play() {
	player->play();
}

void Video::pause() {
	//player->pau
}

int Video::width() {
	return player->width();
}

int Video::height() {
	return player->height();
}

Texture* Video::currentImage() {
	update(0);
	return image;
}

void Video::update(double time) {
	player->video_read(videodata);
	u8* data = image->lock();
	for (int y = 0; y < image->texHeight; ++y) {
		for (int x = 0; x < image->texWidth; ++x) {
			data[y * image->stride() + x * 4 + 0] = videodata[y * width() * 4 + x * 4 + 0];
			data[y * image->stride() + x * 4 + 1] = videodata[y * width() * 4 + x * 4 + 1];
			data[y * image->stride() + x * 4 + 2] = videodata[y * width() * 4 + x * 4 + 2];
			data[y * image->stride() + x * 4 + 3] = 255;
		}
	}
	image->unlock();
}
#endif

#if 0
namespace {
	// Helper; just grab some more compressed bitstream and sync it for page extraction
	int buffer_data(FileReader* in, ogg_sync_state *oy) {
		char *buffer = ogg_sync_buffer(oy, 4096);
		int bytes = in->read(buffer, 4096);
		//int bytes = fread(buffer, 1, 4096, in);
		ogg_sync_wrote(oy, bytes);
		return(bytes);
	}

	// Report the encoder-specified colorspace for the video, if any.
	// We don't actually make use of the information in this example;
	// a real player should attempt to perform color correction for
	// whatever display device it supports.
	void report_colorspace(th_info* ti) {
		switch (ti->colorspace){
		case TH_CS_UNSPECIFIED:
			/* nothing to report */
			break;;
		case TH_CS_ITU_REC_470M:
			fprintf(stderr, "  encoder specified ITU Rec 470M (NTSC) color.\n");
			break;;
		case TH_CS_ITU_REC_470BG:
			fprintf(stderr, "  encoder specified ITU Rec 470BG (PAL) color.\n");
			break;;
		default:
			fprintf(stderr, "warning: encoder specified unknown colorspace (%d).\n",
				ti->colorspace);
			break;;
		}
	}

	// dump the theora (or vorbis) comment header
	int dump_comments(th_comment *tc){
		int i, len;
		char *value;
		FILE *out = stdout;

		fprintf(out, "Encoded by %s\n", tc->vendor);
		if (tc->comments){
			fprintf(out, "theora comment header:\n");
			for (i = 0; i<tc->comments; i++){
				if (tc->user_comments[i]){
					len = tc->comment_lengths[i];
					value = (char*)malloc(len + 1);
					memcpy(value, tc->user_comments[i], len);
					value[len] = '\0';
					fprintf(out, "\t%s\n", value);
					free(value);
				}
			}
		}
		return(0);
	}
}

// helper: push a page into the appropriate steam
// this can be done blindly; a stream won't accept a page that doesn't belong to it
int Video::queue_page(ogg_page* page) {
	if (theora_p) ogg_stream_pagein(&to, page);
	if (vorbis_p) ogg_stream_pagein(&vo, page);
	return 0;
}

Video::Video(const char* filename) : duration(0), position(0), finished(false), paused(false) {
	videoImage = new Image(100, 100, Image::RGBA32, false);

	theora_p = 0;
	vorbis_p = 0;
	stateflag = 0;

	frames = 0;
	dropped = 0;

	videobuf_ready = 0;
	videobuf_granulepos = -1;
	videobuf_time = 0;

	audiobuf_fill = 0;
	audiobuf_ready = 0;
	audiobuf_granulepos = 0;

	ogg_sync_init(&oy);

	/* init supporting Vorbis structures needed in header parsing */
	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);

	/* init supporting Theora structures needed in header parsing */
	th_comment_init(&tc);
	th_info_init(&ti);

	infile = new FileReader(filename);
	int stateflag = 0;
	/* Ogg file open; parse the headers */
	/* Only interested in Vorbis/Theora streams */
	while (!stateflag){
		int ret = buffer_data(infile, &oy);
		if (ret == 0)break;
		while (ogg_sync_pageout(&oy, &og)>0){
			ogg_stream_state test;

			/* is this a mandated initial header? If not, stop parsing */
			if (!ogg_page_bos(&og)){
				/* don't leak the page; get it into the appropriate stream */
				queue_page(&og);
				stateflag = 1;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&og));
			ogg_stream_pagein(&test, &og);
			ogg_stream_packetout(&test, &op);


			/* identify the codec: try theora */
			if (!theora_p && th_decode_headerin(&ti, &tc, &ts, &op) >= 0){
				/* it is theora */
				memcpy(&to, &test, sizeof(test));
				theora_p = 1;
			}
			else if (!vorbis_p && vorbis_synthesis_headerin(&vi, &vc, &op) >= 0){
				/* it is vorbis */
				memcpy(&vo, &test, sizeof(test));
				vorbis_p = 1;
			}
			else{
				/* whatever it is, we don't care about it */
				ogg_stream_clear(&test);
			}
		}
		/* fall through to non-bos page parsing */
	}

	/* we're expecting more header packets. */
	while ((theora_p && theora_p<3) || (vorbis_p && vorbis_p<3)){
		int ret;

		/* look for further theora headers */
		while (theora_p && (theora_p<3) && (ret = ogg_stream_packetout(&to, &op))){
			if (ret<0){
				fprintf(stderr, "Error parsing Theora stream headers; "
					"corrupt stream?\n");
				exit(1);
			}
			if (!th_decode_headerin(&ti, &tc, &ts, &op)){
				fprintf(stderr, "Error parsing Theora stream headers; "
					"corrupt stream?\n");
				exit(1);
			}
			theora_p++;
		}

		/* look for more vorbis header packets */
		while (vorbis_p && (vorbis_p<3) && (ret = ogg_stream_packetout(&vo, &op))){
			if (ret<0){
				fprintf(stderr, "Error parsing Vorbis stream headers; corrupt stream?\n");
				exit(1);
			}
			if (vorbis_synthesis_headerin(&vi, &vc, &op)){
				fprintf(stderr, "Error parsing Vorbis stream headers; corrupt stream?\n");
				exit(1);
			}
			vorbis_p++;
			if (vorbis_p == 3)break;
		}

		/* The header pages/packets will arrive before anything else we
		care about, or the stream is not obeying spec */

		if (ogg_sync_pageout(&oy, &og)>0){
			queue_page(&og); /* demux into the appropriate stream */
		}
		else{
			int ret = buffer_data(infile, &oy); /* someone needs more data */
			if (ret == 0){
				fprintf(stderr, "End of file while searching for codec headers.\n");
				exit(1);
			}
		}
	}

	/* and now we have it all.  initialize decoders */
	if (theora_p){
		td = th_decode_alloc(&ti, ts);
		printf("Ogg logical stream %lx is Theora %dx%d %.02f fps",
			to.serialno, ti.pic_width, ti.pic_height,
			(double)ti.fps_numerator / ti.fps_denominator);
		px_fmt = ti.pixel_fmt;
		switch (ti.pixel_fmt){
		case TH_PF_420: printf(" 4:2:0 video\n"); break;
		case TH_PF_422: printf(" 4:2:2 video\n"); break;
		case TH_PF_444: printf(" 4:4:4 video\n"); break;
		case TH_PF_RSVD:
		default:
			printf(" video\n  (UNKNOWN Chroma sampling!)\n");
			break;
		}
		if (ti.pic_width != ti.frame_width || ti.pic_height != ti.frame_height)
			printf("  Frame content is %dx%d with offset (%d,%d).\n",
			ti.frame_width, ti.frame_height, ti.pic_x, ti.pic_y);
		report_colorspace(&ti);
		dump_comments(&tc);
		th_decode_ctl(td, TH_DECCTL_GET_PPLEVEL_MAX, &pp_level_max,
			sizeof(pp_level_max));
		pp_level = pp_level_max;
		th_decode_ctl(td, TH_DECCTL_SET_PPLEVEL, &pp_level, sizeof(pp_level));
		pp_inc = 0;

		/*{
		int arg = 0xffff;
		th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_MBMODE,&arg,sizeof(arg));
		th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_MV,&arg,sizeof(arg));
		th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_QI,&arg,sizeof(arg));
		arg=10;
		th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_BITS,&arg,sizeof(arg));
		}*/
	}
	else{
		/* tear down the partial theora setup */
		th_info_clear(&ti);
		th_comment_clear(&tc);
	}

	th_setup_free(ts);

	if (vorbis_p){
		vorbis_synthesis_init(&vd, &vi);
		vorbis_block_init(&vd, &vb);
		fprintf(stderr, "Ogg logical stream %lx is Vorbis %d channel %ld Hz audio.\n",
			vo.serialno, vi.channels, vi.rate);
	}
	else{
		/* tear down the partial vorbis setup */
		vorbis_info_clear(&vi);
		vorbis_comment_clear(&vc);
	}
}

Video::~Video() {
	if (vorbis_p){
		ogg_stream_clear(&vo);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}
	if (theora_p){
		ogg_stream_clear(&to);
		th_decode_free(td);
		th_comment_clear(&tc);
		th_info_clear(&ti);
	}
	ogg_sync_clear(&oy);

	//if (infile && infile != stdin)fclose(infile);

	fprintf(stderr,
		"\r                                                             \r");
	fprintf(stderr, "%d frames", frames);
	if (dropped) fprintf(stderr, " (%d dropped)", dropped);
	fprintf(stderr, "\n");
	fprintf(stderr, "\nDone.\n");
}

void Video::update(double time) {
	stateflag = 0; /* playback has not begun */
	//while (!got_sigint){

		// we want a video and audio frame ready to go at all times.  If
		// we have to buffer incoming, buffer the compressed data (ie, let
		// ogg do the buffering)
		/*while (vorbis_p && !audiobuf_ready){
			int ret;
			float **pcm;

			// if there's pending, decoded audio, grab it
			if ((ret = vorbis_synthesis_pcmout(&vd, &pcm))>0){
				int count = audiobuf_fill / 2;
				int maxsamples = (audiofd_fragsize - audiobuf_fill) / 2 / vi.channels;
				for (i = 0; i<ret && i<maxsamples; i++)
				for (j = 0; j<vi.channels; j++){
					int val = rint(pcm[j][i] * 32767.f);
					if (val>32767)val = 32767;
					if (val<-32768)val = -32768;
					audiobuf[count++] = val;
				}
				vorbis_synthesis_read(&vd, i);
				audiobuf_fill += i*vi.channels * 2;
				if (audiobuf_fill == audiofd_fragsize)audiobuf_ready = 1;
				if (vd.granulepos >= 0)
					audiobuf_granulepos = vd.granulepos - ret + i;
				else
					audiobuf_granulepos += i;

			}
			else{

				// no pending audio; is there a pending packet to decode? 
				if (ogg_stream_packetout(&vo, &op)>0){
					if (vorbis_synthesis(&vb, &op) == 0) // test for success!
						vorbis_synthesis_blockin(&vd, &vb);
				}
				else   // we need more data; break out to suck in another page
					break;
			}
		}*/

		while (theora_p && !videobuf_ready){
			/* theora is one in, one out... */
			if (ogg_stream_packetout(&to, &op)>0){

				if (pp_inc){
					pp_level += pp_inc;
					th_decode_ctl(td, TH_DECCTL_SET_PPLEVEL, &pp_level,
						sizeof(pp_level));
					pp_inc = 0;
				}
				/*HACK: This should be set after a seek or a gap, but we might not have
				a granulepos for the first packet (we only have them for the last
				packet on a page), so we just set it as often as we get it.
				To do this right, we should back-track from the last packet on the
				page and compute the correct granulepos for the first packet after
				a seek or a gap.*/
				if (op.granulepos >= 0){
					th_decode_ctl(td, TH_DECCTL_SET_GRANPOS, &op.granulepos,
						sizeof(op.granulepos));
				}
				if (th_decode_packetin(td, &op, &videobuf_granulepos) == 0){
					videobuf_time = th_granule_time(td, videobuf_granulepos);
					frames++;

					/* is it already too old to be useful?  This is only actually
					useful cosmetically after a SIGSTOP.  Note that we have to
					decode the frame even if we don't show it (for now) due to
					keyframing.  Soon enough libtheora will be able to deal
					with non-keyframe seeks.  */

					if (videobuf_time >= time)
						videobuf_ready = 1;
					else{
						/*If we are too slow, reduce the pp level.*/
						pp_inc = pp_level>0 ? -1 : 0;
						dropped++;
					}
				}

			}
			else
				break;
		}

		if (!videobuf_ready && !audiobuf_ready && infile->pos() >= infile->size()) return;

		if (!videobuf_ready || !audiobuf_ready){
			/* no data yet for somebody.  Grab another page */
			buffer_data(infile, &oy);
			while (ogg_sync_pageout(&oy, &og)>0){
				queue_page(&og);
			}
		}

		/* If playback has begun, top audio buffer off immediately. */
		//if (stateflag) audio_write_nonblocking();

		/* are we at or past time for this video frame? */
		if (stateflag && videobuf_ready && videobuf_time <= time) {
			video_write();
			videobuf_ready = 0;
		}

		/*if (stateflag &&
			(audiobuf_ready || !vorbis_p) &&
			(videobuf_ready || !theora_p) //&&
			//!got_sigint
			)
		{
			// we have an audio frame ready (which means the audio buffer is
			// full), it's not time to play video, so wait until one of the
			// audio buffer is ready or it's near time to play video

			// set up select wait on the audiobuffer and a timeout for video
			struct timeval timeout;
			fd_set writefs;
			fd_set empty;
			int n = 0;

			FD_ZERO(&writefs);
			FD_ZERO(&empty);
			if (audiofd >= 0){
				FD_SET(audiofd, &writefs);
				n = audiofd + 1;
			}

			if (theora_p){
				double tdiff;
				long milliseconds;
				tdiff = videobuf_time - get_time();
				// If we have lots of extra time, increase the post-processing level.
				if (tdiff>ti.fps_denominator*0.25 / ti.fps_numerator){
					pp_inc = pp_level<pp_level_max ? 1 : 0;
				}
				else if (tdiff<ti.fps_denominator*0.05 / ti.fps_numerator){
					pp_inc = pp_level>0 ? -1 : 0;
				}
				milliseconds = tdiff * 1000 - 5;
				if (milliseconds>500)milliseconds = 500;
				if (milliseconds>0){
					timeout.tv_sec = milliseconds / 1000;
					timeout.tv_usec = (milliseconds % 1000) * 1000;

					n = select(n, &empty, &writefs, &empty, &timeout);
					if (n)audio_calibrate_timer(0);
				}
			}
			else{
				select(n, &empty, &writefs, &empty, NULL);
			}
		}*/

		/* if our buffers either don't exist or are ready to go,
		we can begin playback */
		if ((!theora_p || videobuf_ready) &&
			(!vorbis_p || audiobuf_ready)) stateflag = 1;
		/* same if we've run out of input */
		if (infile->pos() >= infile->size()) stateflag = 1;

	//}
}

void Video::play() {

}

void Video::pause() {

}

int Video::width() {
	return 100;
}

int Video::height() {
	return 100;
}

Image* Video::currentImage() {
	return videoImage;
}

void Video::video_write() {
	int i;
	th_ycbcr_buffer yuv;
	int y_offset, uv_offset;
	th_decode_ycbcr_out(td, yuv);
	// Lock SDL_yuv_overlay
	//if (SDL_MUSTLOCK(screen)) {
	//	if (SDL_LockSurface(screen) < 0) return;
	//}
	//if (px_fmt != TH_PF_444 && SDL_LockYUVOverlay(yuv_overlay) < 0) return;

	// let's draw the data on a SDL screen (*screen)
	// deal with border stride
	// reverse u and v for SDL
	// and crop input properly, respecting the encoded frame rect
	// problems may exist for odd frame rect for some encodings

	y_offset = (ti.pic_x&~1) + yuv[0].stride*(ti.pic_y&~1);

	/*if (px_fmt == TH_PF_422) {
		uv_offset = (ti.pic_x / 2) + (yuv[1].stride)*(ti.pic_y);
		// SDL doesn't have a planar 4:2:2
		for (i = 0; i<yuv_overlay->h; i++) {
			int j;
			char *in_y = (char *)yuv[0].data + y_offset + yuv[0].stride*i;
			char *out = (char *)(yuv_overlay->pixels[0] + yuv_overlay->pitches[0] * i);
			for (j = 0; j<yuv_overlay->w; j++)
				out[j * 2] = in_y[j];
			char *in_u = (char *)yuv[1].data + uv_offset + yuv[1].stride*i;
			char *in_v = (char *)yuv[2].data + uv_offset + yuv[2].stride*i;
			for (j = 0; j<yuv_overlay->w >> 1; j++) {
				out[j * 4 + 1] = in_u[j];
				out[j * 4 + 3] = in_v[j];
			}
		}
	}
	else if (px_fmt == TH_PF_444){
		//SDL_Surface *output;
		for (i = 0; i<screen->h; i++) {
			int j;
			unsigned char *in_y = (unsigned char *)yuv[0].data + y_offset + yuv[0].stride*i;
			unsigned char *in_u = (unsigned char *)yuv[1].data + y_offset + yuv[1].stride*i;
			unsigned char *in_v = (unsigned char *)yuv[2].data + y_offset + yuv[2].stride*i;
			unsigned char *out = RGBbuffer + (screen->w*i * 4);
			for (j = 0; j<screen->w; j++) {
				int r, g, b;
				r = (1904000 * in_y[j] + 2609823 * in_v[j] - 363703744) / 1635200;
				g = (3827562 * in_y[j] - 1287801 * in_u[j]
					- 2672387 * in_v[j] + 447306710) / 3287200;
				b = (952000 * in_y[j] + 1649289 * in_u[j] - 225932192) / 817600;
				out[4 * j + 0] = OC_CLAMP255(b);
				out[4 * j + 1] = OC_CLAMP255(g);
				out[4 * j + 2] = OC_CLAMP255(r);
			}
			//output = SDL_CreateRGBSurfaceFrom(RGBbuffer, screen->w, screen->h, 32, 4 * screen->w, 0, 0, 0, 0);
			//SDL_BlitSurface(output, NULL, screen, NULL);
		}
	}
	else {
		uv_offset = (ti.pic_x / 2) + (yuv[1].stride)*(ti.pic_y / 2);
		for (i = 0; i<yuv_overlay->h; i++)
			memcpy(yuv_overlay->pixels[0] + yuv_overlay->pitches[0] * i,
			yuv[0].data + y_offset + yuv[0].stride*i,
			yuv_overlay->w);
		for (i = 0; i<yuv_overlay->h / 2; i++){
			memcpy(yuv_overlay->pixels[1] + yuv_overlay->pitches[1] * i,
				yuv[2].data + uv_offset + yuv[2].stride*i,
				yuv_overlay->w / 2);
			memcpy(yuv_overlay->pixels[2] + yuv_overlay->pitches[2] * i,
				yuv[1].data + uv_offset + yuv[1].stride*i,
				yuv_overlay->w / 2);
		}
	}*/
}
#endif
