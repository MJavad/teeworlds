#ifndef ENGINE_CLIENT_DEMORECORDER_H
#define ENGINE_CLIENT_DEMORECORDER_H
#include <engine/demo_recorder.h>
#include <engine/sound.h>
#include <base/math.h>
#include <engine/external/libtheora/theora/theora.h>
#include <engine/external/libtheora/theora/codec.h>
#include <engine/external/libtheora/theora/theoraenc.h>
#include <engine/external/libvorbis/vorbis/vorbisenc.h>

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
    unsigned int x;
    unsigned int y;

    unsigned int x1;
    unsigned int y1;

    unsigned long yuv_w;
    unsigned long yuv_h;

    unsigned char *yuv_y;
    unsigned char *yuv_u;
    unsigned char *yuv_v;

    yuv_w = ycbcr[0].width;
    yuv_h = ycbcr[0].height;

    yuv_y = ycbcr[0].data;
    yuv_u = ycbcr[1].data;
    yuv_v = ycbcr[2].data;

    /*This ignores gamma and RGB primary/whitepoint differences.
      It also isn't terribly fast (though a decent compiler will
      strength-reduce the division to a multiplication).*/

    if (Format == TH_PF_420)
    {
        for(y = 0; y < h; y += 2)
        {
            y1=y+(y+1<h);
            for(x = 0; x < w; x += 2)
            {
                x1=x+(x+1<w);
                unsigned char r0 = pPng[y * w + 3 * x + 0];
                unsigned char g0 = pPng[y * w + 3 * x + 1];
                unsigned char b0 = pPng[y * w + 3 * x + 2];
                unsigned char r1 = pPng[y * w + 3 * x1 + 0];
                unsigned char g1 = pPng[y * w + 3 * x1 + 1];
                unsigned char b1 = pPng[y * w + 3 * x1 + 2];
                unsigned char r2 = pPng[y1 * w + 3 * x + 0];
                unsigned char g2 = pPng[y1 * w + 3 * x + 1];
                unsigned char b2 = pPng[y1 * w + 3 * x + 2];
                unsigned char r3 = pPng[y1 * w + 3 * x1 + 0];
                unsigned char g3 = pPng[y1 * w + 3 * x1 + 1];
                unsigned char b3 = pPng[y1 * w + 3 * x1 + 2];

                yuv_y[x  + y * yuv_w]  = clamp((65481*r0+128553*g0+24966*b0+4207500)/255000, 0, 255);
                yuv_y[x1 + y * yuv_w]  = clamp((65481*r1+128553*g1+24966*b1+4207500)/255000, 0, 255);
                yuv_y[x  + y1 * yuv_w] = clamp((65481*r2+128553*g2+24966*b2+4207500)/255000, 0, 255);
                yuv_y[x1 + y1 * yuv_w] = clamp((65481*r3+128553*g3+24966*b3+4207500)/255000, 0, 255);

                yuv_u[(x >> 1) + (y >> 1) * ycbcr[1].stride] = clamp(((-33488*r0-65744*g0+99232*b0+29032005)/4 + (-33488*r0-65744*g0+99232*b0+29032005)/4 + (-33488*r2-65744*g2+99232*b2+29032005)/4 + (-33488*r3-65744*g3+99232*b3+29032005)/4)/225930, 0, 255);
                yuv_v[(x >> 1) + (y >> 1) * ycbcr[2].stride] = clamp(((157024*r0-131488*g0-25536*b0+45940035)/4 + (157024*r1-131488*g1-25536*b1+45940035)/4 + (157024*r2-131488*g2-25536*b2+45940035)/4 + (157024*r3-131488*g3-25536*b3+45940035)/4)/357510, 0, 255);
            }
        }
    }
    else if (Format == TH_PF_444)
    {
        for(y = 0; y < h; y++)
        {
            for(x = 0; x < w; x++)
            {
                int i = ((h - y) * w + x) * 3;
                unsigned char r = pPng[i + 0];
                unsigned char g = pPng[i + 1];
                unsigned char b = pPng[i + 2];

                yuv_y[x + y * yuv_w] = clamp((65481*r+128553*g+24966*b+4207500)/255000, 0, 255);
                yuv_u[x + y * yuv_w] = clamp((-33488*r-65744*g+99232*b+29032005)/225930, 0, 255);
                yuv_v[x + y * yuv_w] = clamp((157024*r-131488*g-25536*b+45940035)/357510, 0, 255);
            }
        }
    }
    else      /* TH_PF_422 */
    {
        for(y = 0; y < h; y += 1)
        {
            for(x = 0; x < w; x += 2)
            {
                x1=x+(x+1<w);
                unsigned char r0 = pPng[y * w + 3 * x + 0];
                unsigned char g0 = pPng[y * w + 3 * x + 1];
                unsigned char b0 = pPng[y * w + 3 * x + 2];
                unsigned char r1 = pPng[y * w + 3 * x1 + 0];
                unsigned char g1 = pPng[y * w + 3 * x1 + 1];
                unsigned char b1 = pPng[y * w + 3 * x1 + 2];

                yuv_y[x  + y * yuv_w] = clamp((65481*r0+128553*g0+24966*b0+4207500)/255000, 0, 255);
                yuv_y[x1 + y * yuv_w] = clamp((65481*r1+128553*g1+24966*b1+4207500)/255000, 0, 255);

                yuv_u[(x >> 1) + y * ycbcr[1].stride] = clamp(((-33488*r0-65744*g0+99232*b0+29032005)/2 + (-33488*r1-65744*g1+99232*b1+29032005)/2)/225930, 0, 255);
                yuv_v[(x >> 1) + y * ycbcr[2].stride] = clamp(((157024*r0-131488*g0-25536*b0+45940035)/2 + (157024*r1-131488*g1-25536*b1+45940035)/2)/357510, 0, 255);
            }
        }
    }

}


#endif
