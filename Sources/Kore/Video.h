#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>
#include <theora/theoradec.h>
#include <vorbis/codec.h>

class TheoraVideoClip;

namespace Kore {
	class Video {
	public:
		Video(const char* filename);
		~Video();
		void play();
		void pause();
		int width();
		int height();
		Texture* currentImage();
		double duration; //milliseconds
		double position; //milliseconds
		bool finished;
		bool paused;
		void update(double time);

	private:
		TheoraVideoClip* clip;
		Texture* image;
		double lastTime;

	private:
		void video_write();
		int queue_page(ogg_page* page);
		Kore::Image* videoImage;
		ogg_sync_state   oy;
		ogg_page         og;
		ogg_stream_state vo;
		ogg_stream_state to;
		th_info      ti;
		th_comment   tc;
		th_dec_ctx       *td;
		th_setup_info    *ts;
		vorbis_info      vi;
		vorbis_dsp_state vd;
		vorbis_block     vb;
		vorbis_comment   vc;
		th_pixel_fmt     px_fmt;
		int              theora_p;
		int              vorbis_p;
		int              stateflag;
		int frames;
		int dropped;
		int videobuf_ready;
		ogg_int64_t  videobuf_granulepos;
		double       videobuf_time;
		int          audiobuf_fill = 0;
		int          audiobuf_ready = 0;
		ogg_int16_t *audiobuf;
		ogg_int64_t  audiobuf_granulepos = 0;
		int pp_level_max;
		int pp_level;
		int pp_inc;
		int i, j;
		ogg_packet op;
		FileReader* infile;
	};
}
