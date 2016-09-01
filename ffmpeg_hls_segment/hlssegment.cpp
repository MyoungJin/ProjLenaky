/*
 * Example created by muzie (mailto:sjy1937@hotmail.com)
 *
 * */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h> 
#include <libavformat/avformat.h> 
#include <libavformat/avio.h> 
#include <libavdevice/avdevice.h>
}

static const unsigned long long _2_33 = 0x200000000;

int 
main (int argc, char** argv) 
{ 
    avdevice_register_all();
    av_register_all(); 
    avcodec_register_all(); 
    avformat_network_init(); 
    // Initialize

    char szBasePath[512] = {0,};

    if (argc > 1)
    {
        snprintf(szBasePath, sizeof(szBasePath), "%s", argv[1]);
    }

    int vidx = 0, aidx = 0;  // Video, Audio Index
    AVFormatContext* ctx = avformat_alloc_context(); 
    AVDictionary *dicts = NULL;
    AVFormatContext* oc = NULL; 

    std::string strlist = std::string(szBasePath) + std::string("playlist.m3u8");

    avformat_alloc_output_context2(&oc, NULL, "hls", strlist.c_str()); // apple hls. If you just want to segment file use "segment"

    int rc = av_dict_set(&dicts, "rtsp_transport", "tcp", 0); // default udp. Set tcp interleaved mode
    if (rc < 0)
    {
        return EXIT_FAILURE;
    }

    /* 
    rc = av_dict_set(&dicts, "stimeout", "1 * 1000 * 1000", 0); // timeout option
    if (rc < 0)
    {
        return -1;
    }
    */

    //open rtsp 
    if (avformat_open_input(&ctx, "rtsp://211.189.132.118/nbr-media/media.nmp?ch=T142",NULL, &dicts) != 0)
    //if (avformat_open_input(&ctx, "rtsp://user:Vpass2@192.168.100.142/axis-media/media.amp",NULL, &dicts) != 0)
    { 
        return EXIT_FAILURE; 
    } 

    // get context
    if (avformat_find_stream_info(ctx, NULL) < 0)
    {
        return EXIT_FAILURE; 
    } 

    //search video stream , audio stream
    for (int i = 0 ; i < ctx->nb_streams ; i++)
    { 
        if (ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
            vidx = i; 
        if (ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            aidx = i;
    } 

    AVPacket packet; 

    //open output file 
    if (oc == NULL)
    {
        return EXIT_FAILURE;
    }

    AVStream* vstream = NULL; 
    AVStream* astream = NULL;

    vstream = avformat_new_stream(oc, ctx->streams[vidx]->codec->codec); 
    astream = avformat_new_stream(oc, ctx->streams[aidx]->codec->codec);

    avcodec_copy_context(vstream->codec, ctx->streams[vidx]->codec); 
    vstream->time_base = (AVRational){1, 90000};
    vstream->sample_aspect_ratio = ctx->streams[vidx]->codec->sample_aspect_ratio; 

    avcodec_copy_context(astream->codec, ctx->streams[aidx]->codec); 
    astream->time_base = (AVRational){1, 16000}; // 여기서 time base 고정해도 어느순간부터 90000으로 되어있음.

    int cnt = 0; 

    int64_t vprepts = 0;
    int64_t aprepts = 0;
    int64_t voffset = 0;
    int64_t aoffset = 0;

    av_read_play(ctx);//play RTSP 

    int i = (1 << 4); // omit endlist
    int j = (1 << 1);  // delete segment. 
    //  libavformat/hlsenc.c 's description shows that no longer available files will be deleted but it doesnt works as described.

    std::string strseg = std::string(szBasePath) + std::string("file%03d.ts");
    av_opt_set(oc->priv_data, "hls_segment_filename", strseg.c_str(), 0);
    av_opt_set_int(oc->priv_data, "hls_list_size", 5, 0);
    av_opt_set_int(oc->priv_data, "hls_time", 3, 0);
    av_opt_set_int(oc->priv_data, "hls_flags", i|j, 0);

    avformat_write_header(oc, NULL); 

    while (true)
    {
        av_init_packet(&packet); 
        int nRecvPacket = av_read_frame(ctx, &packet);

        if (packet.stream_index == vidx) // video frame
        {
            if (vprepts == 0)
            {
                voffset = 1;
                vprepts = 1;
            }
            else
            {
                if (vprepts < packet.pts)
                {
                    voffset = voffset + (packet.pts-vprepts);
                }
                else
                {
                    // 패킷 pts 역전에 대해서 생각해야함.
                }
                vprepts = packet.pts;
            }

            vstream->id = vidx;
            packet.pts = voffset;
            cnt++;

        }
        else if (packet.stream_index == aidx) // audio frame
        {
            if (aprepts == 0)
            {
                aoffset = 1;
                aprepts = packet.pts;
            }
            else
            {
                if (aprepts < packet.pts)
                    aoffset += (packet.pts-aprepts);
                else
                {
                    // 패킷 역전에 대해서 생각 해야함. (예외 처리 안되어있음)
                }
                aprepts = packet.pts;
            }
            astream->id = aidx;
            long long pts = av_rescale_q(aoffset, (AVRational){1, 16000}, (AVRational){1, 90000});
            // mpeg2ts는 Audio 패킷의 경우 90000 단위로 동작한다. RTP에 실을 때 sample rate으로 동작하는 것과는 상이한 점
            packet.pts = pts;
        }

        packet.dts = packet.pts;// generally, dts is same as pts. it only differ when the stream has b-frame
        av_write_frame(oc,&packet); 
        av_packet_unref(&packet); 

        if (cnt > 30 * 60 * 50) // 30 프레임 - 50분동안.
            break;
    }

    av_read_pause(ctx); 
    av_write_trailer(oc); 
    avformat_free_context(oc); 
    av_dict_free(&dicts);

    return (EXIT_SUCCESS); 
}
