#include "demo_recorder.h"
#include <engine/client.h>
#include <engine/shared/config.h>

void CDemoVideoRecorder::Init(int Width, int Height, int FPS, int Format, const char *pName)
{
    m_pSound = Kernel()->RequestInterface<ISound>();
    m_FPS = FPS;
    m_ScreenWidth = Width;
    m_ScreenHeight = Height;
    m_Format = Format;

    if (m_Format == IClient::DEMO_RECORD_FORMAT_OGV)
    {
        ogg_stream_init(&m_TheoraOggStreamState, rand());
        ogg_stream_init(&m_VorbisOggStreamState, rand());

        char aBuf[1024];
        if (str_find_rev(pName, "/"))
            str_format(aBuf, sizeof(aBuf), "%s.ogv", str_find_rev(pName, "/"));
        else if (str_find_rev(aBuf, "\\"))
            str_format(aBuf, sizeof(pName), "%s.ogv", str_find_rev(pName, "\\"));
        else
            str_format(aBuf, sizeof(aBuf), "%s.ogv", pName);
        m_OggFile = io_open(aBuf, IOFLAG_WRITE);

        //thread_sleep(10000);
        vorbis_info_init(&m_VorbisEncodingInfo);
        vorbis_encode_init_vbr(&m_VorbisEncodingInfo, 2, g_Config.m_SndRate, 1.0f); //2 ch - samplerate - quality 1
        vorbis_analysis_init(&m_VorbisState, &m_VorbisEncodingInfo);
        vorbis_block_init(&m_VorbisState, &m_VorbisBlock);

        vorbis_comment_init(&m_VorbisComment);
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;
        vorbis_analysis_headerout(&m_VorbisState, &m_VorbisComment, &header, &header_comm, &header_code);
        ogg_stream_packetin(&m_VorbisOggStreamState, &header);
        ogg_stream_packetin(&m_VorbisOggStreamState, &header_comm);
        ogg_stream_packetin(&m_VorbisOggStreamState, &header_code);


        th_info_init(&m_TheoraEncodingInfo);
        m_TheoraEncodingInfo.frame_width = m_ScreenWidth+15&~0xF;
        m_TheoraEncodingInfo.frame_height = m_ScreenHeight+15&~0xF;
        m_TheoraEncodingInfo.pic_width = m_ScreenWidth;
        m_TheoraEncodingInfo.pic_height = m_ScreenHeight;
        m_TheoraEncodingInfo.pic_x = m_TheoraEncodingInfo.frame_width - m_ScreenWidth>>1&~1;
        m_TheoraEncodingInfo.pic_y = m_TheoraEncodingInfo.frame_height - m_ScreenHeight>>1&~1;
        m_TheoraEncodingInfo.colorspace = TH_CS_UNSPECIFIED;
        m_TheoraEncodingInfo.fps_numerator = FPS; //fps
        m_TheoraEncodingInfo.fps_denominator = 1;
        m_TheoraEncodingInfo.aspect_numerator = -1;
        m_TheoraEncodingInfo.aspect_denominator = -1;
        m_TheoraEncodingInfo.pixel_fmt = TH_PF_444;
        m_TheoraEncodingInfo.target_bitrate = (int)(64870*(ogg_int64_t)48000>>16);
        m_TheoraEncodingInfo.quality = 32;
        m_TheoraEncodingInfo.keyframe_granule_shift = 0;
        m_pThreoraContext = th_encode_alloc(&m_TheoraEncodingInfo);
        int arg = TH_RATECTL_CAP_UNDERFLOW;
        th_encode_ctl(m_pThreoraContext, TH_ENCCTL_SET_RATE_FLAGS, &arg, sizeof(arg));
        th_comment CommentHeader;
        ogg_packet OggPacket;
        th_comment_init(&CommentHeader);
        mem_zero(&OggPacket, sizeof(OggPacket));


            //Flush
        //Step 1
        th_encode_flushheader(m_pThreoraContext, &CommentHeader, &OggPacket); // first header

        ogg_stream_packetin(&m_TheoraOggStreamState, &OggPacket);
        //
        ogg_page OggPage;
        ogg_stream_pageout(&m_TheoraOggStreamState, &OggPage);
        io_write(m_OggFile, OggPage.header, OggPage.header_len);
        io_write(m_OggFile, OggPage.body, OggPage.body_len);

        while(1)
        {
            ogg_page OggPage;
            if (ogg_stream_flush(&m_VorbisOggStreamState,&OggPage) == 0)
                break;
            io_write(m_OggFile, OggPage.header, OggPage.header_len);
            io_write(m_OggFile, OggPage.body, OggPage.body_len);
        }

        while(th_encode_flushheader(m_pThreoraContext, &CommentHeader, &OggPacket))
        {
            ogg_stream_packetin(&m_TheoraOggStreamState, &OggPacket);
        }

        ogg_stream_flush(&m_TheoraOggStreamState, &OggPage);
        io_write(m_OggFile, OggPage.header, OggPage.header_len);
        io_write(m_OggFile, OggPage.body, OggPage.body_len);
    }
    if (m_Format == IClient::DEMO_RECORD_FORMAT_WEBM)
    {

    }
}

void CDemoVideoRecorder::Stop()
{
    if (m_Format == IClient::DEMO_RECORD_FORMAT_OGV)
    {
        /* need to read the final (summary) packet */
        /*unsigned char aBuffer[65536];
        int Bytes = th_encode_ctl(m_pThreoraContext, TH_ENCCTL_2PASS_OUT, &aBuffer, sizeof(aBuffer));
        io_seek(m_OggFile, 0, IOSEEK_START);
        io_write(m_OggFile, aBuffer, Bytes);*/



        ogg_stream_clear(&m_VorbisOggStreamState);
        vorbis_block_clear(&m_VorbisBlock);
        vorbis_comment_clear(&m_VorbisComment);
        vorbis_info_clear(&m_VorbisEncodingInfo);


        th_encode_free(m_pThreoraContext);
        ogg_stream_clear(&m_TheoraOggStreamState);
        th_info_clear(&m_TheoraEncodingInfo);

        if (m_OggFile)
            io_close(m_OggFile);
        m_OggFile = 0;
    }
}

void CDemoVideoRecorder::OnData(unsigned char *pPixelData, void *pUser)
{
    ((CDemoVideoRecorder *)pUser)->OnFrame(pPixelData);
}

void CDemoVideoRecorder::OnFrame(unsigned char *pPixelData)
{
    if (m_Format == IClient::DEMO_RECORD_FORMAT_OGV)
    {
        //SampleRate / FPS
        int Size = g_Config.m_SndRate / m_FPS;
        short *pStream = new short[Size * 2];
        mem_zero(pStream, sizeof(short) * Size * 2);
        m_pSound->MixHook((short *)pStream, Size);
        float **buffer = vorbis_analysis_buffer(&m_VorbisState, Size);
        for (int i = 0; i < Size; i++)
        {
            buffer[0][i] = pStream[i * 2] / 32768.f;
            buffer[1][i] = pStream[i * 2 + 1] / 32768.f;
        }
        delete []pStream;
        vorbis_analysis_wrote(&m_VorbisState, Size);
        while(vorbis_analysis_blockout(&m_VorbisState, &m_VorbisBlock) == 1)
        {

            vorbis_analysis(&m_VorbisBlock, NULL);
            vorbis_bitrate_addblock(&m_VorbisBlock);

            ogg_packet Packet;

            while(vorbis_bitrate_flushpacket(&m_VorbisState, &Packet))
            {

                ogg_stream_packetin(&m_VorbisOggStreamState, &Packet);

                ogg_page OggPage;
                while(1)
                {
                    if (ogg_stream_flush(&m_VorbisOggStreamState,&OggPage) == 0)
                        break;
                    io_write(m_OggFile, OggPage.header, OggPage.header_len);
                    io_write(m_OggFile, OggPage.body, OggPage.body_len);
                    if (ogg_page_eos(&OggPage))
                        break;
                }
            }
        }

        unsigned char *pTempRow = pPixelData + m_ScreenWidth * m_ScreenHeight * 3;
        static th_ycbcr_buffer ycbcr = {0};
        if (!ycbcr[0].data)
        {
            th_pixel_fmt chroma_format = TH_PF_444;
            ycbcr[0].width = ((m_ScreenWidth + 15) & ~15);
            ycbcr[0].height = ((m_ScreenHeight + 15) & ~15);
            ycbcr[0].stride = ((m_ScreenWidth + 15) & ~15);
            ycbcr[1].width = (chroma_format == TH_PF_444) ? ((m_ScreenWidth + 15) & ~15) : (((m_ScreenWidth + 15) & ~15) >> 1);
            ycbcr[1].stride = ycbcr[1].width;
            ycbcr[1].height = (chroma_format == TH_PF_420) ? (((m_ScreenHeight + 15) & ~15) >> 1) : ((m_ScreenHeight + 15) & ~15);
            ycbcr[2].width = ycbcr[1].width;
            ycbcr[2].stride = ycbcr[1].stride;
            ycbcr[2].height = ycbcr[1].height;

            ycbcr[0].data = (unsigned char *)malloc(ycbcr[0].stride * ycbcr[0].height);
            ycbcr[1].data = (unsigned char *)malloc(ycbcr[1].stride * ycbcr[1].height);
            ycbcr[2].data = (unsigned char *)malloc(ycbcr[2].stride * ycbcr[2].height);

            mem_zero(ycbcr[0].data, ycbcr[0].stride * ycbcr[0].height);
            mem_zero(ycbcr[1].data, ycbcr[1].stride * ycbcr[1].height);
            mem_zero(ycbcr[2].data, ycbcr[2].stride * ycbcr[2].height);
        }
        rgb_to_yuv(pPixelData, ycbcr, m_ScreenWidth, m_ScreenHeight, TH_PF_444);
        th_encode_ycbcr_in(m_pThreoraContext, ycbcr);
        ogg_packet Packet;
        ogg_page OggPage;
        th_encode_packetout(m_pThreoraContext, 0, &Packet);
        ogg_stream_packetin(&m_TheoraOggStreamState, &Packet);
        while(ogg_stream_pageout(&m_TheoraOggStreamState, &OggPage))
        {
            io_write(m_OggFile, OggPage.header, OggPage.header_len);
            io_write(m_OggFile, OggPage.body, OggPage.body_len);
        }
    }
}
