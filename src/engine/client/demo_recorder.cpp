#include "demo_recorder.h"
#include <engine/client.h>
#include <engine/shared/config.h>

int CDemoVideoRecorder::WritePacket(void *opaque, uint8_t *buf, int buf_size)
{
    CDemoVideoRecorder *pSelf = (CDemoVideoRecorder *)opaque;
    dbg_msg("size", "%i", buf_size);
    return io_write(pSelf->m_OutFile, buf, buf_size);
}

void CDemoVideoRecorder::Init(int Width, int Height, int FPS, int Format, const char *pName)
{
    m_Counter = 0;
    m_Frame = 0;

    m_pAudioFrame = 0;
    m_pVideoFrame = 0;

    m_pSound = Kernel()->RequestInterface<ISound>();
    av_register_all();
    avcodec_register_all();
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
        m_OutFile = io_open(aBuf, IOFLAG_WRITE);

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
        //this should not work, as far as i know. but idk how to kill the green bar at the bottom
        m_TheoraEncodingInfo.frame_width = m_ScreenWidth;//+15&~0xF;
        m_TheoraEncodingInfo.frame_height = m_ScreenHeight;//+15&~0xF;
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
        io_write(m_OutFile, OggPage.header, OggPage.header_len);
        io_write(m_OutFile, OggPage.body, OggPage.body_len);

        while(1)
        {
            ogg_page OggPage;
            if (ogg_stream_flush(&m_VorbisOggStreamState,&OggPage) == 0)
                break;
            io_write(m_OutFile, OggPage.header, OggPage.header_len);
            io_write(m_OutFile, OggPage.body, OggPage.body_len);
        }

        while(th_encode_flushheader(m_pThreoraContext, &CommentHeader, &OggPacket))
        {
            ogg_stream_packetin(&m_TheoraOggStreamState, &OggPacket);
        }

        ogg_stream_flush(&m_TheoraOggStreamState, &OggPage);
        io_write(m_OutFile, OggPage.header, OggPage.header_len);
        io_write(m_OutFile, OggPage.body, OggPage.body_len);
    }
    if (m_Format == IClient::DEMO_RECORD_FORMAT_WEBM)
    {
        char aBuf[1024];
        if (str_find_rev(pName, "/"))
            str_format(aBuf, sizeof(aBuf), "%s.mov", str_find_rev(pName, "/"));
        else if (str_find_rev(aBuf, "\\"))
            str_format(aBuf, sizeof(pName), "%s.mov", str_find_rev(pName, "\\"));
        else
            str_format(aBuf, sizeof(aBuf), "%s.mov", pName);

        m_pOutputFormat = av_guess_format(NULL, ".mov", NULL);
        dbg_msg("Audio", "Got: %i - Wanted: %i", m_pOutputFormat->audio_codec, AV_CODEC_ID_MP3);
        dbg_msg("Audio", "Got: %i - Wanted: %i", m_pOutputFormat->video_codec, AV_CODEC_ID_H264);
        m_pOutputFormat->audio_codec = AV_CODEC_ID_MP3;
        m_pOutputFormat->video_codec = AV_CODEC_ID_H264;
        m_pOutputFormat->subtitle_codec = AV_CODEC_ID_NONE;
        m_pOutputFormat->flags = AVFMT_GLOBALHEADER;

        m_pFormatContext = avformat_alloc_context();
        if (!m_pFormatContext)
        {
            dbg_msg("Record", "Error opening output file");
            //stop!
        }
        m_pFormatContext->oformat = m_pOutputFormat;
        str_copy(m_pFormatContext->filename, aBuf, sizeof(m_pFormatContext->filename));

        AVCodec *AudioCodec = avcodec_find_encoder(m_pOutputFormat->audio_codec);
        AVCodec *VideoCodec = avcodec_find_encoder(m_pOutputFormat->video_codec);

        m_pAudioStream = avformat_new_stream(m_pFormatContext, AudioCodec);
        m_pAudioEncoder = m_pAudioStream->codec;
        m_pAudioEncoder->coder_type = AVMEDIA_TYPE_AUDIO;

        if (m_pFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        {
            m_pAudioEncoder->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
        m_pAudioEncoder->bit_rate = 0;
        m_pAudioEncoder->sample_rate = g_Config.m_SndRate;
        m_pAudioEncoder->channels = 2;
        m_pAudioEncoder->channel_layout = AV_CH_LAYOUT_STEREO;
        if (m_pOutputFormat->audio_codec == AV_CODEC_ID_MP3)
            m_pAudioEncoder->sample_fmt = AV_SAMPLE_FMT_S16P;
        if (m_pOutputFormat->audio_codec == AV_CODEC_ID_AAC)
            m_pAudioEncoder->sample_fmt = AV_SAMPLE_FMT_S16;

        int Err = 0;
        Err = avcodec_open2(m_pAudioEncoder, AudioCodec, 0);
        if (Err)
        {
            dbg_msg("Record", "Error while starting the audio codec: %i", Err);
        }

        m_pVideoStream = avformat_new_stream(m_pFormatContext, VideoCodec);
        m_pVideoEncoder = m_pVideoStream->codec;
        m_pVideoEncoder->coder_type = AVMEDIA_TYPE_VIDEO;

        if (m_pFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        {
            m_pVideoEncoder->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
        m_pVideoEncoder->width = m_ScreenWidth;
        m_pVideoEncoder->height = m_ScreenHeight;
        m_pVideoEncoder->pix_fmt = AV_PIX_FMT_YUV444P;
        m_pVideoEncoder->time_base.den = m_FPS;
        m_pVideoEncoder->time_base.num = 1;
        m_pVideoEncoder->max_b_frames = 1;
        m_pVideoEncoder->gop_size = 10;
        m_pVideoEncoder->bit_rate = 400000;

        Err = avcodec_open2(m_pVideoEncoder, VideoCodec, 0);
        if (Err)
        {
            dbg_msg("Record", "Error while starting the video codec: %i", Err);
        }

        if (!(m_pFormatContext->flags & AVFMT_NOFILE))
        {
            Err = avio_open(&m_pFormatContext->pb, aBuf, AVIO_FLAG_WRITE);

            /*m_OutFile = io_open(aBuf, IOFLAG_WRITE);
            Err = avio_open(&m_pFormatContext->pb, 0, AVIO_FLAG_WRITE);
            m_pFormatContext->pb->opaque = this;
            m_pFormatContext->pb->write_packet = WritePacket;
            m_pFormatContext->pb->write_flag = AVIO_FLAG_WRITE;*/

            if (Err)
            {
                dbg_msg("Record", "Error opening file: %i", Err);
            }
        }

        //av_dump_format(m_pFormatContext, 0, aBuf, 1);
        Err = avformat_write_header(m_pFormatContext, 0);
        if (Err)
        {
            dbg_msg("Record", "Error while writing header: %i", Err);
        }
    }
}

void CDemoVideoRecorder::Stop()
{
    if (m_Format == IClient::DEMO_RECORD_FORMAT_OGV)
    {
        /* need to read the final (summary) packet */
        /*unsigned char aBuffer[65536];
        int Bytes = th_encode_ctl(m_pThreoraContext, TH_ENCCTL_2PASS_OUT, &aBuffer, sizeof(aBuffer));
        io_seek(m_OutFile, 0, IOSEEK_START);
        io_write(m_OutFile, aBuffer, Bytes);*/



        ogg_stream_clear(&m_VorbisOggStreamState);
        vorbis_block_clear(&m_VorbisBlock);
        vorbis_comment_clear(&m_VorbisComment);
        vorbis_info_clear(&m_VorbisEncodingInfo);


        th_encode_free(m_pThreoraContext);
        ogg_stream_clear(&m_TheoraOggStreamState);
        th_info_clear(&m_TheoraEncodingInfo);

    }
    else
    {
        int GotOutput = 1;
        while(GotOutput)
        {
            AVPacket Packet;
            av_init_packet(&Packet);
            Packet.data = 0;
            Packet.size = 0;
            Packet.pts = m_Counter++;
            Packet.dts = m_Counter;
            if (m_pVideoEncoder->coded_frame->key_frame)
                Packet.flags |= AV_PKT_FLAG_KEY;
            Packet.stream_index = m_pVideoStream->index;

            m_pVideoFrame->pts = m_Counter;
            avcodec_encode_video2(m_pVideoEncoder, &Packet, 0, &GotOutput);
            Packet.pts = m_Counter;
            Packet.dts = m_Counter;
            if (GotOutput)
            {
                av_interleaved_write_frame(m_pFormatContext, &Packet);
                av_free_packet(&Packet);
            }
        }
        av_write_trailer(m_pFormatContext);

        avcodec_close(m_pAudioEncoder);
        av_free(m_pAudioEncoder);
        avcodec_free_frame(&m_pAudioFrame);

        avcodec_close(m_pVideoEncoder);
        av_free(m_pVideoEncoder);
        av_freep(m_pVideoFrame->data);
        avcodec_free_frame(&m_pVideoFrame);
    }

    if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
    {
        avio_close(m_pFormatContext->pb);
    }
}

void CDemoVideoRecorder::OnData(unsigned char *pPixelData, void *pUser)
{
    ((CDemoVideoRecorder *)pUser)->OnFrame(pPixelData);
}

void CDemoVideoRecorder::OnFrame(unsigned char *pPixelData)
{
    m_Frame++;
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
                    io_write(m_OutFile, OggPage.header, OggPage.header_len);
                    io_write(m_OutFile, OggPage.body, OggPage.body_len);
                    if (ogg_page_eos(&OggPage))
                        break;
                }
            }
        }

        static th_ycbcr_buffer ycbcr = {0};
        if (!ycbcr[0].data)
        {
            th_pixel_fmt chroma_format = TH_PF_444;
            ycbcr[0].width = ((m_ScreenWidth + 15) & ~15);
            ycbcr[0].height = ((m_ScreenHeight + 15) & ~15);
            ycbcr[0].stride = ((m_ScreenWidth + 15) & ~15);
            ycbcr[1].width = ((m_ScreenWidth + 15) & ~15);
            ycbcr[1].stride = ycbcr[1].width;
            ycbcr[1].height = ((m_ScreenHeight + 15) & ~15);
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
            io_write(m_OutFile, OggPage.header, OggPage.header_len);
            io_write(m_OutFile, OggPage.body, OggPage.body_len);
        }
    }
    else
    {
        int Size = g_Config.m_SndRate / m_FPS;
        short *pStream = new short[Size * 2];
        mem_zero(pStream, sizeof(short) * Size * 2);
        m_pSound->MixHook((short *)pStream, Size);
        m_AudioBuffer.Add((char *)pStream, Size * 2 * sizeof(short));
        delete []pStream;
        int BufferSize = av_samples_get_buffer_size(0, m_pAudioEncoder->channels, m_pAudioEncoder->frame_size, m_pAudioEncoder->sample_fmt, 0);
        while (m_AudioBuffer.GetSize() > BufferSize)
        {
            //void *pSamples = av_malloc(BufferSize * 2);
            char *pTmpBuffer = new char[BufferSize];
            char *pSamples = new char[BufferSize];
            mem_zero(pTmpBuffer, sizeof(char) * BufferSize);
            //void *pSamples = mem_alloc(BufferSize * 2, 0);
            m_AudioBuffer.Get(pTmpBuffer, BufferSize);
            m_AudioBuffer.Remove(BufferSize);
            for (int i = 0; i < BufferSize; i++)
            {
                if (i % 4 == 0)
                {
                    pSamples[i / 2] = pTmpBuffer[i];
                }
                if (i % 4 == 1)
                {
                    pSamples[i / 2 + 1] = pTmpBuffer[i];
                }
                if (i % 4 == 2)
                {
                    pSamples[i / 2 + (BufferSize - 1) / 2] = pTmpBuffer[i];
                }
                if (i % 4 == 3)
                {
                    pSamples[i / 2 + (BufferSize - 1) / 2 + 1] = pTmpBuffer[i];
                }
            }
            if (!m_pAudioFrame)
            {
                m_pAudioFrame = avcodec_alloc_frame();
                m_pAudioFrame->nb_samples = m_pAudioEncoder->frame_size;
                m_pAudioFrame->format = m_pAudioEncoder->sample_fmt;
                m_pAudioFrame->channel_layout = m_pAudioEncoder->channel_layout;
            }
            avcodec_fill_audio_frame(m_pAudioFrame, m_pAudioEncoder->channels, m_pAudioEncoder->sample_fmt, (uint8_t *)pSamples, BufferSize, 0);

            AVPacket Packet;
            av_init_packet(&Packet);

            Packet.data = 0;
            Packet.size = 0;
            Packet.pts = m_Counter++;
            Packet.dts = m_Counter;
            if (m_pAudioEncoder->coded_frame->key_frame)
                Packet.flags |= AV_PKT_FLAG_KEY;
            Packet.stream_index = m_pAudioStream->index;

            int GotOutput = 0;
            int Err = avcodec_encode_audio2(m_pAudioEncoder, &Packet, m_pAudioFrame, &GotOutput);
            Packet.pts = m_Counter;
            Packet.dts = m_Counter;
            if (GotOutput)
            {
                //av_interleaved_write_frame(m_pFormatContext, &Packet);
                av_free_packet(&Packet);
            }
            delete []pSamples;
        }

        if (!m_pVideoFrame)
        {
            m_pVideoFrame = avcodec_alloc_frame();
            m_pVideoFrame->format = m_pVideoEncoder->pix_fmt;
            m_pVideoFrame->width = m_pVideoEncoder->width;
            m_pVideoFrame->height = m_pVideoEncoder->height;
            av_image_alloc(m_pVideoFrame->data, m_pVideoFrame->linesize, m_ScreenWidth, m_ScreenHeight, m_pVideoEncoder->pix_fmt, 32);
        }

        AVPacket Packet;
        av_init_packet(&Packet);

        m_Counter = m_Counter + 10;
        Packet.data = 0;
        Packet.size = 0;
        Packet.pts = m_Counter++;
        Packet.dts = m_Counter;
        if (m_pVideoEncoder->coded_frame->key_frame)
            Packet.flags |= AV_PKT_FLAG_KEY;
        Packet.stream_index = m_pVideoStream->index;

        m_pVideoFrame->pts = m_Counter;

        rgb_to_yuv(pPixelData, m_pVideoFrame, m_ScreenWidth, m_ScreenHeight);
        int GotOutput = 0;
        avcodec_encode_video2(m_pVideoEncoder, &Packet, m_pVideoFrame, &GotOutput);

        Packet.pts = m_Counter;
        Packet.dts = m_Counter;
        if (GotOutput)
        {
            av_interleaved_write_frame(m_pFormatContext, &Packet);
            av_free_packet(&Packet);
        }
    }
}
