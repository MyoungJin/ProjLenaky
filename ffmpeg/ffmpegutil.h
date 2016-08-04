#include "ffmpegwrap.h"
#include "ffmpegdef.h"
#include <string>
#include <exception>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

namespace syncview {
namespace ffmpegutil {

struct ffctx
{
    struct SwsContext* ctx;
    unsigned int nSW;
    unsigned int nSH;
    raw_fmt_t    nST;
    unsigned int nDW;
    unsigned int nDH;
    raw_fmt_t    nDT;
};

struct ffrgb
{
    char r;
    char g;
    char b;
};

struct ffmergectx
{
    unsigned int nBGIW;
    unsigned int nBGIH;

    unsigned int nFGIW;
    unsigned int nFGIH;

    unsigned int nX;
    unsigned int nY;

    raw_fmt_t nRawfmt;

    unsigned char* pRawRect;
    unsigned int nRawRectSize;

    unsigned char nMargin;
};

typedef enum 
{
    SAMPLING_96000HZ = 0,
    SAMPLING_88200HZ = 1,
    SAMPLING_64000HZ = 2,
    SAMPLING_48000HZ = 3,
    SAMPLING_44100HZ = 4,
    SAMPLING_32000HZ = 5,
    SAMPLING_24000HZ = 6,
    SAMPLING_22050HZ = 7,
    SAMPLING_16000HZ = 8,
    SAMPLING_12000HZ = 9,
    SAMPLING_11025HZ = 10,
    SAMPLING_8000HZ  = 11,
    SAMPLING_7350HZ  = 12,
} mpeg4_audio_sampling_t;

typedef enum
{
    AUDIO_AAC_MAIN = 1,
    AUDIO_AAC_LC   = 2,
} mpeg4_audio_object_t;

typedef enum
{
    CHANNEL_1CH = 1,
    CHANNEL_2CH = 2,
    CHANNEL_3CH = 3,
    CHANNEL_4CH = 4,
    CHANNEL_5CH = 5,
    CHANNEL_6CH = 6,
    CHANNEL_8CH = 7,
} mpeg4_audio_channel_t;

// defined at http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio

typedef struct _auconfigpkt
{
    unsigned char samplerate_lo:3;
    unsigned char objecttype:5;
    
    unsigned char extflag:1;
    unsigned char depends_on_core_coder:1;
    unsigned char framelength:1;
    unsigned char channel:4;
    unsigned char samplerate_hi:1;
} auconfigpkt_t;

typedef union {
    auconfigpkt_t au;
    char buffer[2];
} AUCONFIG;

class FUtil : std::exception 
{

    public :

        static struct ffmergectx* CreateMergeCtx(unsigned int nBGIW, unsigned int nBGIH,
                unsigned int nFGIW, unsigned int nFGIH, raw_fmt_t nRawfmt, 
                unsigned int& nX, unsigned int& nY,
                unsigned char nMargin, char r, char g, char b);

        static void FreeMergeCtx(struct ffmergectx* pCtx);

        static int MergeImage(struct ffmergectx* pCtx,
                unsigned char* pBGData, unsigned int nBGLength,
                unsigned char* pFGData, unsigned int nFGLength,
                unsigned char* pOutputBuffer, unsigned int nOutputSize);

        static bool IsAvailRawType(raw_fmt_t t);
        static int MergeImageRGB24(unsigned char* pBGData, unsigned int nBGLength,
                unsigned int nBGIW, unsigned int nBGIH,
                unsigned char* pFGData, unsigned int nFGLength, 
                unsigned int nFGIW, unsigned int nFGIH, 
                unsigned int nX, unsigned int nY, unsigned int nMargin,
                unsigned char* pOutputBuffer, unsigned int nOutputSize);
        static int MergeImageYUV420P(unsigned char* pBGData, unsigned int nBGLength,
                unsigned int nBGIW, unsigned int nBGIH,
                unsigned char* pFGData, unsigned int nFGLength, 
                unsigned int nFGIW, unsigned int nFGIH, 
                unsigned int nX, unsigned int nY, unsigned int nMargin, unsigned char* pMargin, unsigned int nMarginLength,
                unsigned char* pOutputBuffer, unsigned int nOutputSize);
        static AVPixelFormat ConvertPxFmt(raw_fmt_t val);
        static unsigned int GuessSize(raw_fmt_t val, unsigned int nW, unsigned int nH);

        // Resize Function. 
        static struct ffctx* CreateSwsCtx(unsigned int nSW, unsigned int nSH, raw_fmt_t nST,
                                unsigned int nDW, unsigned int nDH, raw_fmt_t nDT);
        static void FreeCtx(struct ffctx* ctx);
        static int GetBPP(raw_fmt_t t);
        static int ResizeImage(struct ffctx* ctx, unsigned char* pData, unsigned char* pBuffer);
        
        static void CreateAACHeader(unsigned int nChannel, unsigned int nSamplerate, std::string strAACType, unsigned char* pBuffer);

        // first, call GetResizeCtx and get swscontext and store it.
        // second, get BPP(Byte Per Pixel)
        // third, Do Resize Image
        // fourth, release ctx.


    private :
        FUtil(){};


};

}  // end of namespace ffmpegutil
}  // end of namespace syncview