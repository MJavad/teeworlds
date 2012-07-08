#include "demo_recorder.h"

void CDemoVideoRecorder::Init(int Width, int Height)
{
    m_pSound = Kernel()->RequestInterface<ISound>();
    if (m_ScreenWidth)
    {
        ogg_stream_destroy(&m_TheoraOggStreamState);
        ogg_stream_destroy(&m_VorbisOggStreamState);
        th_encode_free(m_pContext);

        if (m_OggFile)
            io_close(m_OggFile);
    }
    m_ScreenWidth = Width;
    m_ScreenHeight = Height;
    ogg_stream_init(&m_TheoraOggStreamState, rand());
    ogg_stream_init(&m_VorbisOggStreamState, rand());

    m_OggFile = io_open("test.ogg", IOFLAG_WRITE);

    //thread_sleep(10000);
    static vorbis_info VorbisEncodingInfo;
    vorbis_info_init(&VorbisEncodingInfo);
    vorbis_encode_init_vbr(&VorbisEncodingInfo, 2, 44100, 1.0f);
    vorbis_analysis_init(&m_VorbisState, &VorbisEncodingInfo);
    vorbis_block_init(&m_VorbisState, &m_VorbisBlock);

    vorbis_comment VorbisComment;
    vorbis_comment_init(&VorbisComment);
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
    vorbis_analysis_headerout(&m_VorbisState, &VorbisComment, &header, &header_comm, &header_code);
    ogg_stream_packetin(&m_VorbisOggStreamState, &header);
    ogg_stream_packetin(&m_VorbisOggStreamState, &header_comm);
    ogg_stream_packetin(&m_VorbisOggStreamState, &header_code);


    th_info TheoraEncodingInfo;
    th_info_init(&TheoraEncodingInfo);
    TheoraEncodingInfo.frame_width = m_ScreenWidth+15&~0xF;
    TheoraEncodingInfo.frame_height = m_ScreenHeight+15&~0xF;
    TheoraEncodingInfo.pic_width = m_ScreenWidth;
    TheoraEncodingInfo.pic_height = m_ScreenHeight;
    TheoraEncodingInfo.pic_x = TheoraEncodingInfo.frame_width - m_ScreenWidth>>1&~1;
    TheoraEncodingInfo.pic_y = TheoraEncodingInfo.frame_height - m_ScreenHeight>>1&~1;
    TheoraEncodingInfo.colorspace = TH_CS_UNSPECIFIED;
    TheoraEncodingInfo.fps_numerator = 25; //fps
    TheoraEncodingInfo.fps_denominator = 1;
    TheoraEncodingInfo.aspect_numerator = -1;
    TheoraEncodingInfo.aspect_denominator = -1;
    TheoraEncodingInfo.pixel_fmt = TH_PF_444;
    TheoraEncodingInfo.target_bitrate = (int)(64870*(ogg_int64_t)480000>>16);
    TheoraEncodingInfo.quality = 63;
    TheoraEncodingInfo.keyframe_granule_shift = 0;
    m_pContext = th_encode_alloc(&TheoraEncodingInfo);
    //int arg = TH_RATECTL_CAP_UNDERFLOW;
    //th_encode_ctl(m_pContext, TH_ENCCTL_SET_RATE_FLAGS, &arg, sizeof(arg));
    th_info_clear(&TheoraEncodingInfo);
    th_comment CommentHeader;
    ogg_packet OggPacket;
    th_comment_init(&CommentHeader);
    mem_zero(&OggPacket, sizeof(OggPacket));
    dbg_msg("Context pointer", "%p", m_pContext);


        //Flush

    /*while(1)
    {
        ogg_page OggPage;
        if (ogg_stream_flush(&m_VorbisOggStreamState,&OggPage) == 0)
            break;
        io_write(m_OggFile, OggPage.header, OggPage.header_len);
        io_write(m_OggFile, OggPage.body, OggPage.body_len);
    }*/


    //Step 1
    th_encode_flushheader(m_pContext, &CommentHeader, &OggPacket); // first header
    ogg_stream_packetin(&m_TheoraOggStreamState, &OggPacket);

    //
    ogg_page OggPage;
    ogg_stream_pageout(&m_TheoraOggStreamState, &OggPage);
    io_write(m_OggFile, OggPage.header, OggPage.header_len);
    io_write(m_OggFile, OggPage.body, OggPage.body_len);

    while(th_encode_flushheader(m_pContext, &CommentHeader, &OggPacket))
    {
        ogg_stream_packetin(&m_TheoraOggStreamState, &OggPacket);
    }

    ogg_stream_flush(&m_TheoraOggStreamState, &OggPage);
    io_write(m_OggFile, OggPage.header, OggPage.header_len);
    io_write(m_OggFile, OggPage.body, OggPage.body_len);
}

void CDemoVideoRecorder::OnData(unsigned char *pPixelData, void *pUser)
{
    dbg_msg("Got", "Frame");
    ((CDemoVideoRecorder *)pUser)->OnFrame(pPixelData);
}

void CDemoVideoRecorder::OnFrame(unsigned char *pPixelData)
{
    /*static int Test = 0;
    Test = Test + 3528;
    if (Test >= 4096)
    {
        Test = 0;
        short aStream[4096] = {0};
        m_pSound->MixHook((short *)aStream, sizeof(aStream) / 2 / 2);
        float **buffer = vorbis_analysis_buffer(&m_VorbisState, sizeof(aStream) / 2 / 2);
        for (int i = 0; i < sizeof(aStream) / 2 / 2; i++)
        {
            buffer[0][i] = aStream[i * 2] / 32768.f;
            buffer[1][i] = aStream[i * 2 + 1] / 32768.f;
        }
        vorbis_analysis_wrote(&m_VorbisState, sizeof(aStream) / 2 / 2);
        while(vorbis_analysis_blockout(&m_VorbisState, &m_VorbisBlock)==1)
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
    }*/

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
    }
    rgb_to_yuv(pPixelData, ycbcr, m_ScreenWidth, m_ScreenHeight, TH_PF_444);
    th_encode_ycbcr_in(m_pContext, ycbcr);
    ogg_packet Packet;
    ogg_page OggPage;
    th_encode_packetout(m_pContext, 0, &Packet);
    ogg_stream_packetin(&m_TheoraOggStreamState, &Packet);
    while(ogg_stream_pageout(&m_TheoraOggStreamState, &OggPage))
    {
        io_write(m_OggFile, OggPage.header, OggPage.header_len);
        io_write(m_OggFile, OggPage.body, OggPage.body_len);
    }
}
