#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h> 
#include <libavformat/avformat.h> 
#include <libavformat/avio.h> 
#include <libavdevice/avdevice.h>
}

int 
main (void) 
{ 
    avdevice_register_all();
    av_register_all(); 
    avcodec_register_all(); 
    avformat_network_init(); 
    // Initialize

    int vidx = 0, aidx = 0;  // Video, Audio Index
    AVFormatContext* ctx = avformat_alloc_context(); 
    AVDictionary *dicts = NULL;
    AVFormatContext* oc = NULL; 
    avformat_alloc_output_context2(&oc, NULL, "mp4", NULL);

    int rc = av_dict_set(&dicts, "rtsp_transport", "tcp", 0); // default udp. Set tcp interleaved mode
    if (rc < 0)
    {
        return EXIT_FAILURE;
    }

    /*
    rc = av_dict_set(&dicts, "stimeout", "1 * 1000 * 1000", 0);
    if (rc < 0)
    {
        printf("errrrrrrr    %d\n", rc);
        return -1;
    }
    */

    //open rtsp 
    if (avformat_open_input(&ctx, "rtsp://211.189.132.118:4554/nbr-media/media.nmp?ch=T142",NULL, &dicts) != 0)
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
        printf("failed\n");
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
    astream->time_base = (AVRational){1, 16000};

    int cnt = 0; 
    long long nGap = 0;
    long long nGap2 = 1;

    av_read_play(ctx);//play RTSP 

    avio_open(&oc->pb, "./output.mkv", AVIO_FLAG_WRITE); 
    avformat_write_header(oc, NULL); 

    while (true)
    {
        av_init_packet(&packet); 
        int nRecvPacket = av_read_frame(ctx, &packet);

        if (packet.stream_index == vidx) // video frame
        {
            vstream->id = vidx;
            packet.pts = av_rescale_q(nGap, (AVRational){1, 10000}, oc->streams[packet.stream_index]->time_base);
            nGap += 333;
            cnt++;

        }
        else if (packet.stream_index == aidx) // audio frame
        {
            astream->id = aidx;
            packet.pts = av_rescale_q(nGap2, (AVRational){1, 10000}, oc->streams[packet.stream_index]->time_base);
            nGap2 += 666;
        }

        packet.dts = packet.pts;// generally, dts is same as pts. it only differ when the stream has b-frame
        av_write_frame(oc,&packet); 
        av_packet_unref(&packet); 

        if (cnt > 300) // 10 sec
            break;
    }

    av_read_pause(ctx); 
    av_write_trailer(oc); 
    avio_close(oc->pb); 
    avformat_free_context(oc); 
    av_dict_free(&dicts);

    return (EXIT_SUCCESS); 
}
