#ifndef ENGINE_CLIENT_DEMORECORDER_H
#define ENGINE_CLIENT_DEMORECORDER_H
#include <engine/demo_recorder.h>
#include <engine/sound.h>
#include <base/math.h>
#include <engine/external/libtheora/theora/theora.h>
#include <engine/external/libtheora/theora/codec.h>
#include <engine/external/libtheora/theora/theoraenc.h>
#include <engine/external/libvorbis/vorbis/vorbisenc.h>
#include <time.h>
class CDemoVideoRecorder : public IDemoVideoRecorder
{
    /*lib theora*/
    th_info m_TheoraEncodingInfo;
    vorbis_info m_VorbisEncodingInfo;
    vorbis_comment m_VorbisComment;

    th_enc_ctx *m_pThreoraContext;
    vorbis_dsp_state m_VorbisState;
    vorbis_block m_VorbisBlock;
    ogg_stream_state m_TheoraOggStreamState;
    ogg_stream_state m_VorbisOggStreamState;
    IOHANDLE m_OggFile;
public:
    void Init(int Width, int Height, int FPS, int Format, const char *pName);
    void Stop();
    void OnFrame(unsigned char *pPixelData);
    static void OnData(unsigned char *pPixelData, void *pUser);

    int m_ScreenWidth;
    int m_ScreenHeight;
    int m_FPS;
    int m_Format;

    ISound *m_pSound;
};


static void rgb_to_yuv(const unsigned char *pPng, th_ycbcr_buffer ycbcr, unsigned int w, unsigned int h, th_pixel_fmt Format)
{
	int64 time = time_get();
    unsigned int x;   

    unsigned char *yuv_y = ycbcr[0].data;
    unsigned char *yuv_u = ycbcr[1].data;
    unsigned char *yuv_v = ycbcr[2].data;

    /*This ignores gamma and RGB primary/whitepoint differences.
      It also isn't terribly fast (though a decent compiler will
      strength-reduce the division to a multiplication).*/

    //Not Fast Enough...
    if (Format == TH_PF_444)
    {
		//for(int times = 0; times < 60; times++)
		{
		int i = h * w * 3;
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		yuv_y = ycbcr[0].data;
		yuv_u = ycbcr[1].data;
		yuv_v = ycbcr[2].data;		
		for(x = 0;;++x)
		{
			if(x == w)
			{
				if(i==w*3)
					break;
				i -= w*6;
				x = 0;						
			}					
			r = pPng[i++];
			g = pPng[i++];
			b = pPng[i++];
			*yuv_y++ = (65481*r+128553*g+24966*b+4207500)/255000;
			*yuv_u++ = (-33488*r-65744*g+99232*b+29032005)/225930;
			*yuv_v++ = (157024*r-131488*g-25536*b+45940035)/357510;
		}		}			
    }
		//dbg_msg("duration", "%f",(float)(60.0f/(float)( (float)(time_get()-time) / (float)time_freq())));
}


#endif
