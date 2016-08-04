#ifndef KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGENC_H_
#define KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGENC_H_

#include "ffmpegwrap.h"
#include "ffmpegdef.h"
#include <string>
#include <exception>
#include <boost/algorithm/string.hpp>

namespace syncview {
namespace ffmpegenc {

typedef enum {
    ffmpeg_prof_not_decided = FF_PROFILE_UNKNOWN,
    ffmpeg_prof_baseline = FF_PROFILE_H264_BASELINE,
    ffmpeg_prof_main = FF_PROFILE_H264_MAIN,
    ffmpeg_prof_extend = FF_PROFILE_H264_EXTENDED,

} h264_prof_t;

class FEncoder : std::exception {
public:
    FEncoder(std::string strTargetCodec, h264_prof_t prof, unsigned int w, unsigned int h,
            raw_fmt_t t, unsigned int bitrate, unsigned int fps);
    virtual ~FEncoder();
    int Initialize(std::string strTargetCodec, h264_prof_t prof, unsigned int w, unsigned int h,
            raw_fmt_t t, unsigned int bitrate, unsigned int fps);
    int GetExtraData(char** ppData, unsigned int& nSize);
    int Encode(unsigned char* pData, unsigned int nDataLength,
                        unsigned char** ppBuffer, unsigned int& nEncodedSize, bool& bIsIDR);
    int Encode(unsigned char* pData, unsigned int nDataLength,
                        unsigned int& nEncodedSize, AVPacket& outpkt, bool& bIsIDR);
    AVCodecID GetCodecID(std::string strCodecName);
    AVCodecContext* GetAVCodecContext(void) { return m_context; }

private:
    FEncoder(){};
    void Clear();
    // to make api user don't let use default constructor
    int Configuration(std::string strCodec);
    int GetProfIndex(h264_prof_t t);
    char* m_sprops;
    unsigned int m_spropssize;
    // sps pps
    std::string         m_codecname;
    h264_prof_t         m_proftype;
    unsigned int        m_bitrate;
    unsigned int        m_imgwidth, m_imgheight;
    unsigned int        m_fps;
    raw_fmt_t           m_rawtype;

    AVCodec*            m_codec;
    AVCodecContext*     m_context;
    AVFrame*            m_frame;
    AVDictionary*       m_dict;

    // information for encoding

    char* m_encodedbuf; // encoded data buffer;
    unsigned int m_enbufsize;
    unsigned int m_enbufrealsize;

};

}; // end of namespace ffmpegenc
}; // end of namespace syncview
#endif
