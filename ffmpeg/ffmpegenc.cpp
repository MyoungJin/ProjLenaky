#include <iostream>
#include "ffmpegenc.h"
#include "ffmpegutil.h"

#pragma warning(disable:4996)

namespace syncview {
namespace ffmpegenc {

FEncoder::FEncoder(std::string strTargetCodec, h264_prof_t prof, unsigned int w, unsigned int h,
            raw_fmt_t t, unsigned int bitrate, unsigned int fps) :
            m_sprops(NULL), m_encodedbuf(NULL), m_spropssize(0),
            m_codecname(strTargetCodec),
            m_proftype(prof), m_bitrate(bitrate),
            m_imgwidth(w), m_imgheight(h), m_fps(fps),
            m_codec(NULL), m_context(NULL), m_frame(NULL), m_dict(NULL),
            m_rawtype(t), m_enbufsize(0), m_enbufrealsize(0)
{
    if (Initialize(strTargetCodec, prof, w, h, t, bitrate, fps)) goto error;
    return ;

error:
    Clear();

    std::cout << "error!" << std::endl;
    throw std::runtime_error(std::string("avcodec error"));
}

int FEncoder::Initialize(std::string strTargetCodec, h264_prof_t prof, unsigned int w,
        unsigned int h, raw_fmt_t t, unsigned int bitrate, unsigned int fps)
{
    Clear();
    m_codecname = strTargetCodec;
    m_proftype = prof;
    m_bitrate = bitrate;
    m_imgwidth = w;
    m_imgheight = h;
    m_fps = fps;
    m_rawtype = t;

    int nRet = 0;

    if (boost::iequals(strTargetCodec, std::string("h264_nvenc")))
    {
        std::cout << "nvenc~" << std::endl;
        m_codec = avcodec_find_encoder_by_name("h264_nvenc");
    }

    if (boost::iequals(strTargetCodec, std::string("h264")))
        m_codec = avcodec_find_encoder(AV_CODEC_ID_H264);

    if (boost::iequals(strTargetCodec, std::string("mpeg4")))
        m_codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);

    m_context = avcodec_alloc_context3(m_codec);
    m_frame = av_frame_alloc();

    if (m_context == NULL || m_frame == NULL)
    {
        return -1;
    }

    if (boost::iequals(strTargetCodec, std::string("h264_nvenc")))
    {
        /*
        add_preset("default"); // 3.7 ~ 3.8段
	    add_preset("hq");  // 5段
	    add_preset("hp"); // 3.6段
	    add_preset("bd"); // 5段
	    add_preset("ll"); // 5段
	    add_preset("llhq"); // 6.3段
	    add_preset("llhp"); // 3.78段 
        */
        av_opt_set(m_context->priv_data, "preset", "llhp", 0);
        av_opt_set_int(m_context->priv_data, "cbr", true, 0);

        av_opt_set(m_context->priv_data, "profile", "baseline", 0);
        av_opt_set(m_context->priv_data, "level", "auto", 0); 
        av_opt_set_int(m_context->priv_data, "2pass", false, 0); // true/false
       // av_opt_set_int(m_context->priv_data, "gpu", 1, 0); // 0~8
        
        m_context->time_base.num = 1;
        m_context->time_base.den = m_fps;
        m_context->gop_size = m_fps;
        m_context->pix_fmt = (AVPixelFormat)ffmpegutil::FUtil::ConvertPxFmt(m_rawtype);
        m_context->width = m_imgwidth;
        m_context->height = m_imgheight;
        m_context->bit_rate = bitrate;
        m_context->rc_buffer_size = bitrate;
        m_context->colorspace = AVCOL_SPC_BT709;
        m_context->color_range = AVCOL_RANGE_MPEG;

        m_context->rc_max_rate = bitrate;
        m_context->rc_min_rate = bitrate;
    }
    else 
    {
        nRet = Configuration(strTargetCodec);
        if (nRet < 0) return -2;
        // configuration
    }

    ffmpegwrap::ffmpeginitialize &inst =  ffmpegwrap::ffmpeginitialize::get_mutable_instance();
    inst.lock();
    nRet = avcodec_open2(m_context, m_codec, NULL);
    inst.unlock();

    if (nRet < 0)
    {   
        char errstring[512] = { 0, };
        av_make_error_string(errstring, sizeof(errstring), nRet);
        std::cout << "open error : " << errstring << std::endl;
        return -3;
    }


    if (boost::iequals(strTargetCodec, std::string("h264_nvenc")))
    {
        m_frame->format = m_context->pix_fmt;
        m_frame->width = m_context->width;
        m_frame->height = m_context->height;
        m_frame->colorspace = m_context->colorspace;
        m_frame->color_range = m_context->color_range;
    }
    else if (boost::iequals(strTargetCodec, std::string("h264")))
    {
        m_spropssize = m_context->extradata_size;
        m_sprops = (char*)malloc(m_spropssize);
        memcpy(m_sprops, m_context->extradata, m_spropssize);
        m_frame->pts = 0;
    }
    else
    {
        m_frame->pts = AV_NOPTS_VALUE;
    }

    m_encodedbuf = (char*)malloc(sizeof(char)*m_imgwidth*m_imgheight*3/2);
    m_enbufsize = m_imgwidth*m_imgheight*3/2;

    if (m_encodedbuf == NULL) return -4;

    return 0;
}

int FEncoder::Configuration(std::string strCodec)
{
    if (boost::iequals(strCodec, std::string("h264")))
    {
        m_context->time_base.num = 1;
        m_context->time_base.den = m_fps;
        m_context->codec_type    = AVMEDIA_TYPE_VIDEO;
        m_context->codec_id      = GetCodecID(m_codecname);
        m_context->gop_size      = m_fps;
        m_context->pix_fmt       = (AVPixelFormat)ffmpegutil::FUtil::ConvertPxFmt(m_rawtype);
        m_context->max_b_frames  = 0;
        m_context->width         = m_imgwidth;
        m_context->height        = m_imgheight;
//        m_context->keyint_min    = m_fps;

        m_frame->width           = m_imgwidth;
        m_frame->height          = m_imgheight;
        m_frame->format          = (AVPixelFormat)ffmpegutil::FUtil::ConvertPxFmt(m_rawtype);

        m_context->qmin          = 10;
        m_context->qmax          = 51;
        m_context->max_qdiff     = 4;
        m_context->bit_rate       = m_bitrate;
        m_context->flags         |= CODEC_FLAG_GLOBAL_HEADER;
        m_context->profile       = m_proftype;

    }

    return 0;
}

AVCodecID FEncoder::GetCodecID(std::string strCodecName)
{
    AVCodecID cd = AV_CODEC_ID_NONE;
    if (boost::iequals(strCodecName, std::string("h264")))
    {
        cd = AV_CODEC_ID_H264;
    }
    // not yet
    return cd;
}

void FEncoder::Clear()
{
    if (m_context)
    {
        ffmpegwrap::ffmpeginitialize &inst =  ffmpegwrap::ffmpeginitialize::get_mutable_instance();
        inst.lock();
        avcodec_close(m_context);
        inst.unlock();
        av_free(m_context);
    }
    if (m_frame)
        av_frame_free(&m_frame);
    if (m_sprops != NULL)
        free(m_sprops);
    if (m_encodedbuf)
        free(m_encodedbuf);

    m_encodedbuf = NULL;
    m_enbufsize = 0;
    m_enbufrealsize = 0;
    m_spropssize = 0;
    m_context = NULL;
    m_frame = NULL;
    m_dict = NULL;
}

int FEncoder::GetExtraData(char** ppData, unsigned int& nSize)
{
    if (m_sprops != NULL)
    {
        *ppData = (char*)m_sprops;
        nSize = m_spropssize;
    }
    else
        return -1;
    return 0;
}

int FEncoder::Encode(unsigned char* pData, unsigned int nDataLength,
                        unsigned char** ppBuffer, unsigned int& nEncodedSize, bool& bIsIDR)
{
    int nPredictSize = avpicture_get_size(ffmpegutil::FUtil::ConvertPxFmt(m_rawtype), m_imgwidth, m_imgheight);
    if (nPredictSize > nDataLength)
        return -1;

    int got_output = 1;
    int nRes = 0;
    AVPacket outpkt;
    av_init_packet(&outpkt);
    outpkt.data = NULL;
    outpkt.size = 0;

    if (pData != NULL)
    {
        if (m_rawtype == ffmpeg_raw_yuv420p)
        {
            unsigned int YSize = m_imgwidth * m_imgheight;
            m_frame->data[0] = pData;
            m_frame->data[1] = m_frame->data[0] + YSize;
            m_frame->data[2] = m_frame->data[1] + YSize / 4;

            m_frame->linesize[0] = m_context->width;
            m_frame->linesize[1] = m_context->width / 2;
            m_frame->linesize[2] = m_context->width / 2;
        }
        else if (m_rawtype == ffmpeg_raw_rgb24)
        {
            m_frame->data[0] = pData;
            m_frame->data[1] = NULL;
            m_frame->data[2] = NULL;

            m_frame->linesize[0] = m_context->width*3;
            m_frame->linesize[1] = NULL;
            m_frame->linesize[2] = NULL;
        }
        else
        {
            //av_packet_unref(&outpkt);
            av_free_packet(&outpkt);
            return -4;
        }

        nRes = avcodec_encode_video2(m_context, &outpkt, m_frame, &got_output);
        ++m_frame->pts;
    }
    else
    {
        nRes = avcodec_encode_video2(m_context, &outpkt, NULL, &got_output);
    }

    nEncodedSize = 0;

    if (nRes == 0 && got_output ==1)
    {
        if (outpkt.size > m_enbufsize)
        {
            free(m_encodedbuf);
            m_encodedbuf = (char*)malloc(sizeof(char)*outpkt.size);
            m_enbufsize = outpkt.size;
        }
        memcpy(m_encodedbuf, outpkt.data, outpkt.size);
        *ppBuffer = (unsigned char*)m_encodedbuf;
        nEncodedSize = outpkt.size;
        m_enbufrealsize = outpkt.size;
        nRes = outpkt.size;
        bIsIDR = m_context->coded_frame->pict_type == AV_PICTURE_TYPE_I ? true : false;
    }

    //av_packet_unref(&outpkt);
    av_free_packet(&outpkt);
    return nRes;
}

int FEncoder::Encode(unsigned char* pData, unsigned int nDataLength, unsigned int& nEncodedSize, AVPacket& outpkt, bool& bIsIDR)
{
    int nPredictSize = avpicture_get_size(ffmpegutil::FUtil::ConvertPxFmt(m_rawtype), m_imgwidth, m_imgheight);
    if (nPredictSize > nDataLength)
        return -1;

    int got_output = 1;
    int nRes = 0;
    outpkt.data = NULL;
    outpkt.size = 0;

    if (pData != NULL)
    {
        if (m_rawtype == ffmpeg_raw_yuv420p)
        {
            unsigned int YSize = m_imgwidth * m_imgheight;
            m_frame->data[0] = pData;
            m_frame->data[1] = m_frame->data[0] + YSize;
            m_frame->data[2] = m_frame->data[1] + YSize / 4;

            m_frame->linesize[0] = m_context->width;
            m_frame->linesize[1] = m_context->width / 2;
            m_frame->linesize[2] = m_context->width / 2;
        }
        else if (m_rawtype == ffmpeg_raw_rgb24)
        {
            m_frame->data[0] = pData;
            m_frame->data[1] = NULL;
            m_frame->data[2] = NULL;

            m_frame->linesize[0] = m_context->width*3;
            m_frame->linesize[1] = NULL;
            m_frame->linesize[2] = NULL;
        }
        else
        {
            return -4;
        }

        nRes = avcodec_encode_video2(m_context, &outpkt, m_frame, &got_output);
        ++m_frame->pts;
    }
    else
    {
        nRes = avcodec_encode_video2(m_context, &outpkt, NULL, &got_output);
    }

    if (nRes == 0 && got_output ==1)
    {
        nEncodedSize = outpkt.size;
        m_enbufrealsize = outpkt.size;
        nRes = outpkt.size;

        bIsIDR = m_context->coded_frame->pict_type == AV_PICTURE_TYPE_I ? true : false;
    }

    return nRes;
}

FEncoder::~FEncoder()
{
    Clear();

}

} // end of namespace ffmpegenc
};
