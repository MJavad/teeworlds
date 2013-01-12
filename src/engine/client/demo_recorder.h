#ifndef ENGINE_CLIENT_DEMORECORDER_H
#define ENGINE_CLIENT_DEMORECORDER_H
#include <engine/demo_recorder.h>
#include <engine/sound.h>
#include <base/math.h>
#include <engine/external/libtheora/theora/theora.h>
#include <engine/external/libtheora/theora/codec.h>
#include <engine/external/libtheora/theora/theoraenc.h>
#include <engine/external/libvorbis/vorbis/vorbisenc.h>
#include <engine/shared/stream.h>
#include <stdint.h>
#include <inttypes.h>
extern "C"
{
    #define UINT64_C(val) val##ULL
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include "libavutil/audioconvert.h"
    #include "libavutil/common.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/samplefmt.h"
}
class CDemoVideoRecorder : public IDemoVideoRecorder
{
    /*libavcodec*/
    AVOutputFormat *m_pOutputFormat;
    AVFormatContext *m_pFormatContext;

    AVStream *m_pVideoStream;
    AVStream *m_pAudioStream;

    AVCodecContext *m_pAudioEncoder;
    AVCodecContext *m_pVideoEncoder;

    AVFrame *m_pAudioFrame;
    AVFrame *m_pVideoFrame;

    CStream m_AudioBuffer;

    static int WritePacket(void *opaque, uint8_t *buf, int buf_size);

    /*lib theora*/
    th_info m_TheoraEncodingInfo;
    vorbis_info m_VorbisEncodingInfo;
    vorbis_comment m_VorbisComment;

    th_enc_ctx *m_pThreoraContext;
    vorbis_dsp_state m_VorbisState;
    vorbis_block m_VorbisBlock;
    ogg_stream_state m_TheoraOggStreamState;
    ogg_stream_state m_VorbisOggStreamState;

    /* Out File*/
    IOHANDLE m_OutFile;
public:
    void Init(int Width, int Height, int FPS, int Format, const char *pName);
    void Stop();
    void OnFrame(unsigned char *pPixelData);
    static void OnData(unsigned char *pPixelData, void *pUser);

    int m_ScreenWidth;
    int m_ScreenHeight;
    int m_FPS;
    int m_Format;


    int m_Frame;
    int m_Counter;

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
    //now it's faster but not fast enough ;)

    //
    if (Format == TH_PF_444)
    {
        int i = h * w * 3;
        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;
        for(x = 0, y = 0;;++x)
        {
            if(x == w)
            {
                i = (h - y) * w * 3;
                x = 0;
                ++y;
                if(y == h)
                    break;
            }
            r = pPng[i++];
            g = pPng[i++];
            b = pPng[i++];
            *yuv_y++ = (65481*r+128553*g+24966*b+4207500)/255000;
            *yuv_u++ = (-33488*r-65744*g+99232*b+29032005)/225930;
            *yuv_v++ = (157024*r-131488*g-25536*b+45940035)/357510;
        }
    }
}

static void rgb_to_yuv(const unsigned char *pPng, AVFrame *pPicture, unsigned int w, unsigned int h)
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

    yuv_y = pPicture->data[0];
    yuv_u = pPicture->data[1];
    yuv_v = pPicture->data[2];

    //now it's faster but not fast enough ;)

    //
    int i = h * w * 3;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    for(x = 0, y = 0;;++x)
    {
        if(x == w)
        {
            i = (h - y) * w * 3;
            x = 0;
            ++y;
            if(y == h)
                break;
        }
        r = pPng[i++];
        g = pPng[i++];
        b = pPng[i++];
        *yuv_y++ = (65481*r+128553*g+24966*b+4207500)/255000;
        *yuv_u++ = (-33488*r-65744*g+99232*b+29032005)/225930;
        *yuv_v++ = (157024*r-131488*g-25536*b+45940035)/357510;
    }
}

#endif
