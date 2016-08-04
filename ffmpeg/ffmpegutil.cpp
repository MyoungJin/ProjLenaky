#include <iostream>
#include "ffmpegutil.h"

#define RNDTO2(X) ( ( (X) & 0xFFFFFFFE ) )
#define RNDTO32(X) ( ( (X) % 32 ) ? ( ( (X) + 32 ) & 0xFFFFFFE0 ) : (X) )

namespace syncview {
namespace ffmpegutil {

struct ffmergectx* FUtil::CreateMergeCtx(unsigned int nBGIW, unsigned int nBGIH,
                unsigned int nFGIW, unsigned int nFGIH, raw_fmt_t nRawfmt,
                unsigned int& nX, unsigned int& nY,
                unsigned char nMargin, char r, char g, char b)
{

    if (nBGIW < (nFGIW + nX + nMargin*2)) 
    {
        nX = nMargin+1;    
    }
    
    if (nBGIH < (nFGIH + nY + nMargin*2))
    {
        nY = nMargin+1;
    }
    
    if (nBGIW < nFGIW) return NULL;
    if (nBGIH < nFGIH) return NULL;
    // foreground image should be smaller than background image.
    struct ffmergectx* pCtx = NULL;
    struct ffrgb rgb;
    pCtx = (struct ffmergectx*)malloc(sizeof(struct ffmergectx));

    if (pCtx == NULL)
        return NULL;

    pCtx->nBGIW = nBGIW;
    pCtx->nBGIH = nBGIH;
    pCtx->nFGIW = nFGIW;
    pCtx->nFGIH = nFGIH;
    pCtx->nRawfmt = nRawfmt;
    pCtx->nX = nX;
    pCtx->nY = nY;
    pCtx->nMargin = nMargin;
    if (nRawfmt == ffmpeg_raw_yuv420p)
    {
        pCtx->nX = nX/2*2;
        pCtx->nY = nY/2*2;
        if (pCtx->nMargin < 2)
            pCtx->nMargin = 2;
        else pCtx->nMargin = pCtx->nMargin/2*2;
    }

    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    
    unsigned int nTmpRectWidth = pCtx->nMargin*2 + nFGIW;
    unsigned int nTmpRectHeight = pCtx->nMargin*2 + nFGIH;

    struct ffctx* c = NULL;
    unsigned char* pTmpBuffer = NULL;
    unsigned char* pYUVBuffer = NULL;

    pTmpBuffer = (unsigned char*)malloc(nTmpRectHeight*nTmpRectWidth*3); // 테두리

    if (pTmpBuffer == NULL) goto err;
    // rgb is source because this is only i know
    for (int  i = 0 ; i < nTmpRectHeight*nTmpRectWidth ; i++)
        memcpy(pTmpBuffer + i*sizeof(struct ffrgb), &rgb, sizeof(struct ffrgb));
    // create rgb rectangle

    if (pCtx->nRawfmt == ffmpeg_raw_rgb24)
    {
        pCtx->pRawRect = pTmpBuffer;
        pCtx->nRawRectSize = nFGIW*nFGIH*3;
        return pCtx; // ok status
    }
    else if (pCtx->nRawfmt == ffmpeg_raw_yuv420p)
    {
        c = CreateSwsCtx(nTmpRectWidth, nTmpRectHeight, ffmpeg_raw_rgb24, 
                         nTmpRectWidth, nTmpRectHeight, ffmpeg_raw_yuv420p);
                         
        pYUVBuffer = (unsigned char*)malloc(nTmpRectHeight*nTmpRectWidth*3/2);
        if (c == NULL || pYUVBuffer == NULL)
            goto err;

        ResizeImage(c, pTmpBuffer, pYUVBuffer);
        FreeCtx(c);

        pCtx->pRawRect = pYUVBuffer;
        pCtx->nRawRectSize = nTmpRectHeight*nTmpRectWidth*3/2;
        free(pTmpBuffer);

        return pCtx; // ok status
    }

err:
    if (pTmpBuffer)
        free(pTmpBuffer);
    if (c)
        FreeCtx(c);
    if (pCtx)
        free(pCtx);
    return NULL;

}

int FUtil::MergeImage(struct ffmergectx* pCtx,
                      unsigned char* pBGData, unsigned int nBGLength,
                      unsigned char* pFGData, unsigned int nFGLength,
                      unsigned char* pOutputBuffer, unsigned int nOutputSize)
{
    int nRet = 0;
    unsigned int nExpectedBGSize = GuessSize(pCtx->nRawfmt, pCtx->nBGIW, pCtx->nBGIH);
    unsigned int nExpectedFGSize = GuessSize(pCtx->nRawfmt, pCtx->nFGIW, pCtx->nFGIH);

    if (nBGLength < nExpectedBGSize) return -100;
    if (nFGLength < nExpectedFGSize) return -200;

    if (pCtx->nRawfmt == ffmpeg_raw_rgb24)
    {
        return MergeImageRGB24(pBGData, nBGLength,
                pCtx->nBGIW, pCtx->nBGIH,
                pFGData, nFGLength,
                pCtx->nFGIW, pCtx->nFGIH,
                pCtx->nX, pCtx->nY, pCtx->nMargin,
                pOutputBuffer, nOutputSize);
    }
    else if (pCtx->nRawfmt == ffmpeg_raw_yuv420p)
    {
         return MergeImageYUV420P(pBGData, nBGLength,
                pCtx->nBGIW, pCtx->nBGIH,
                pFGData, nFGLength,
                pCtx->nFGIW, pCtx->nFGIH,
                pCtx->nX, pCtx->nY, pCtx->nMargin, pCtx->pRawRect, pCtx->nRawRectSize,
                pOutputBuffer, nOutputSize);
    }
    else return -400;
}

void FUtil::FreeMergeCtx(struct ffmergectx* pCtx)
{
    if (pCtx != NULL)
    {
        if (pCtx->pRawRect != NULL)
        {
            free(pCtx->pRawRect);
        }

        free(pCtx);
    }
}

int FUtil::MergeImageRGB24(unsigned char* pBGData, unsigned int nBGLength,
        unsigned int nBGIW, unsigned int nBGIH,
        unsigned char* pFGData, unsigned int nFGLength,
        unsigned int nFGIW, unsigned int nFGIH,
        unsigned int nX, unsigned int nY, unsigned int nMargin, unsigned char* pOutputBuffer, unsigned int nOutputSize)
{
    // img should be rgb24 img
    //
    memcpy(pOutputBuffer, pBGData, nBGIW*nBGIH*3); // image that will be merged is rgb24
    // first copy background image to buffer

    int nFHIndex = 0;
    for (int h = nY ; h < nBGIH ; h++)
    {
        memcpy(pOutputBuffer + h*nBGIW*3 + nX*3, pFGData + nFHIndex*nFGIW*3, nFGIW*3);
        nFHIndex++;
        if (nFHIndex == nFGIH)
            break;
    }
    return 0;
}

void MergeYUV420P(unsigned char* pBGData, unsigned int nBGLength,
        unsigned int nBGIW, unsigned int nBGIH,
        unsigned char* pFGData, unsigned int nFGLength,
        unsigned int nFGIW, unsigned int nFGIH,
        unsigned int nX, unsigned int nY)
{
    unsigned char* pOutputBuffer = pBGData;
    int nFHIndex = 0;
    unsigned char* pOutputU = pOutputBuffer + nBGIW*nBGIH;
    unsigned char* pOutputV = pOutputBuffer + nBGIW*nBGIH + nBGIW/2 * nBGIH/2;

    unsigned char* pFGIU = pFGData + nFGIW*nFGIH;
    unsigned char* pFGIV = pFGIU + nFGIW/2 * nFGIH/2;

    for (int h = nY ; h < nBGIH ; h++)
    {
        memcpy(pOutputBuffer + h*nBGIW + nX, pFGData + nFHIndex*nFGIW, nFGIW);
        nFHIndex++;

        if (nFHIndex == nFGIH)
            break;
    } // copy Y'


    nFHIndex = 0;

    for (int h = nY/2 ; h < nBGIH/2 ; h++)
    {
        memcpy(pOutputU + h*nBGIW/2 + nX/2, pFGIU + nFHIndex*nFGIW/2, nFGIW/2);
        nFHIndex++;

        if (nFHIndex == nFGIH/2)
            break;
    } // copy U'


    nFHIndex = 0;

    for (int h = nY/2 ; h < nBGIH/2 ; h++)
    {
        memcpy(pOutputV + h*nBGIW/2 + nX/2, pFGIV + nFHIndex*nFGIW/2, nFGIW/2);
        nFHIndex++;

        if (nFHIndex == nFGIH/2)
            break;
    } // copy V'    
}

int FUtil::MergeImageYUV420P(unsigned char* pBGData, unsigned int nBGLength,
        unsigned int nBGIW, unsigned int nBGIH,
        unsigned char* pFGData, unsigned int nFGLength,
        unsigned int nFGIW, unsigned int nFGIH,
        unsigned int nX, unsigned int nY, unsigned int nMargin, unsigned char* pMargin, unsigned int nMarginLength, 
        unsigned char* pOutputBuffer, unsigned int nOutputSize)
{
    MergeYUV420P(pMargin, nMarginLength, nFGIW+(nMargin*2), nFGIH+(nMargin*2), 
                    pFGData, nFGLength, nFGIW, nFGIH, nMargin, nMargin);
        
    MergeYUV420P(pBGData, nBGLength, nBGIW, nBGIH, pMargin, 
                   nMarginLength, nFGIW+(nMargin*2), nFGIH+(nMargin*2), nX - nMargin, nY - nMargin);
    
    if (pOutputBuffer != NULL)
        memcpy(pOutputBuffer, pBGData, nOutputSize);
        
    return 0;
}

unsigned int FUtil::GuessSize(raw_fmt_t t, unsigned int nW, unsigned int nH)
{
    int nSize = 0;

    if (nW == 0 || nH == 0)
        return 0;

    switch (t)
    {
        case ffmpeg_raw_rgb24:
            nSize = nW*nH*3;
            break;
        case ffmpeg_raw_rgb565:
            nSize = nW*nH*2;
            break;
        case ffmpeg_raw_bgra:
            nSize = nW*nH*4;
            break;
        case ffmpeg_raw_yuv420p:
            nSize = nW*nH*3/2;
            break;
    }
    return nSize;
}

bool FUtil::IsAvailRawType(raw_fmt_t t)
{
    bool bRet = false;
    if (t < ffmpeg_raw_rgb24 || t > ffmpeg_raw_yuv420p)
        bRet = false;
    else bRet = true;
    return bRet;
}

struct ffctx* FUtil::CreateSwsCtx(unsigned int nSW, unsigned int nSH, raw_fmt_t nST,
                                unsigned int nDW, unsigned int nDH, raw_fmt_t nDT)
{
    if (nSW == 0 || nSH == 0 || nDW == 0 || nDH == 0)
        return NULL;

    struct ffctx* ctx = (struct ffctx*)malloc(sizeof(struct ffctx));
    if (ctx == NULL) return NULL;

    struct SwsContext* res = NULL;
    if (IsAvailRawType(nST) == false || IsAvailRawType(nDT) == false)
        return NULL;

    res = sws_getContext(nSW, nSH, ConvertPxFmt(nST), nDW, nDH, ConvertPxFmt(nDT),
                        SWS_BICUBIC, NULL, NULL, NULL);
    //res = sws_getContext(nSW, nSH, ConvertPxFmt(nST), nDW, nDH, ConvertPxFmt(nDT),
    //                    SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (res == NULL)
        return NULL;

    ctx->ctx = res;
    ctx->nSW = nSW;
    ctx->nSH = nSH;
    ctx->nDW = nDW;
    ctx->nDH = nDH;
    ctx->nST = nST;
    ctx->nDT = nDT;

    return ctx;
}

void FUtil::FreeCtx(struct ffctx* ctx)
{
    if (ctx)
    {
        sws_freeContext(ctx->ctx);
        free(ctx);
    }
}

int FUtil::GetBPP(raw_fmt_t t)
{
    int nSize = 0;
    switch (t)
    {
        case ffmpeg_raw_rgb24:
            nSize = 3;
            break;
        case ffmpeg_raw_rgb565:
            nSize = 2;
            break;
        case ffmpeg_raw_bgra:
            nSize = 4;
            break;
        case ffmpeg_raw_yuv420p:
            nSize = 1;
            break;
    }
    return nSize;
}

int FUtil::ResizeImage(struct ffctx* ctx, unsigned char* pData, unsigned char* pBuffer)
{
    int nSrcBPP = GetBPP(ctx->nST);
    int nDstBPP = GetBPP(ctx->nDT);
    int rgba_stride[3] = {(int)(ctx->nSW)*nSrcBPP, 0, 0};
    uint8_t* rgba_src[3] = {pData, NULL, NULL};
    int rgba_stride_dst[3] = {(int)(ctx->nDW)*nDstBPP, 0, 0};
    uint8_t* rgba_dst[3] = {pBuffer, NULL, NULL};

    int* psrc_stride = NULL;
    uint8_t** ppsrc_data = NULL;

    int* pdst_stride = NULL;
    uint8_t** ppdst_data = NULL;

    int src_ystride  = ctx->nSW;
    int src_uvstride = ctx->nSW / 2;
    int src_ysize    = src_ystride * ctx->nSH;
    int src_vusize   = src_uvstride * (ctx->nSH / 2);
    int src_size     = src_ysize + (2 * src_vusize);

    uint8_t *src_plane[4] = {pData, pData + src_ysize, pData + src_ysize + src_vusize, 0 };
    int src_stride[] = { src_ystride, src_uvstride, src_uvstride, 0 };

    int dst_ystride  = ctx->nDW;
    int dst_uvstride = ctx->nDW / 2;
    int dst_ysize    = dst_ystride * ctx->nDH;
    int dst_vusize   = dst_uvstride * (ctx->nDH / 2);
    int dst_size     = dst_ysize + (2 * dst_vusize);

    uint8_t *dst_plane[4] = {pBuffer, pBuffer + dst_ysize, pBuffer + dst_ysize + dst_vusize, 0 };
    int dst_stride[] = { dst_ystride, dst_uvstride, dst_uvstride, 0 };

    if (ctx->nST == ffmpeg_raw_rgb24)
    {
        psrc_stride = rgba_stride;
        ppsrc_data = rgba_src;
    }
    else if (ctx->nST == ffmpeg_raw_yuv420p)
    {
        psrc_stride = src_stride;
        ppsrc_data = src_plane;
    }
    else
    {
        return -1;
    }

    if (ctx->nDT == ffmpeg_raw_rgb24)
    {
        pdst_stride = rgba_stride_dst;
        ppdst_data = rgba_dst;
    }
    else if (ctx->nDT == ffmpeg_raw_yuv420p)
    {
        pdst_stride = dst_stride;
        ppdst_data = dst_plane;
    }
    else
    {
        return -2;
    }

    if (ctx->ctx)
    {
        //sws_scale(ctx->ctx, rgba_src, rgba_stride, 0, ctx->nSH, rgba_dst, rgba_stride_dst);
        sws_scale(ctx->ctx, ppsrc_data, psrc_stride, 0, ctx->nSH, ppdst_data, pdst_stride);
        //sws_scale(ctx->ctx, rgba_src, rgba_stride, 0, ctx->nSH, dst_plane, dst_stride);
    }
    else
    {
        return -3;
    }
    return 0;
}

AVPixelFormat FUtil::ConvertPxFmt(raw_fmt_t val)
{
    AVPixelFormat nRet = AV_PIX_FMT_NONE;
    switch (val)
    {
        case ffmpeg_raw_rgb24:
            nRet = AV_PIX_FMT_RGB24;
            break;
        case ffmpeg_raw_rgb565:
            nRet = AV_PIX_FMT_RGB565;
            break;
        case ffmpeg_raw_bgra:
            nRet = AV_PIX_FMT_BGRA;
            break;
        case ffmpeg_raw_yuv420p:
            nRet = AV_PIX_FMT_YUV420P;
            break;
        default:
            break;
    }

    return nRet;
}

void FUtil::CreateAACHeader(unsigned int nChannel, unsigned int nSamplerate, std::string strAACType, unsigned char* pBuffer)
{
    AUCONFIG au;
    memset(au.buffer, 0x00, sizeof(AUCONFIG));
    
    mpeg4_audio_sampling_t sample;
    mpeg4_audio_object_t objt;
    mpeg4_audio_channel_t chan;
    
    switch (nSamplerate)
    {
        case 7350:
        sample = SAMPLING_7350HZ; break;
        case 8000:
        sample = SAMPLING_8000HZ; break;
        case 11025:
        sample = SAMPLING_11025HZ; break;
        case 12000:
        sample = SAMPLING_12000HZ; break;
        case 16000:
        sample = SAMPLING_16000HZ; break;
        case 22050:
        sample = SAMPLING_22050HZ; break;
        case 24000:
        sample = SAMPLING_24000HZ; break;
        case 32000:
        sample = SAMPLING_32000HZ; break;
        case 44100:
        sample = SAMPLING_44100HZ; break;
        case 48000:
        sample = SAMPLING_48000HZ; break;
        case 64000:
        sample = SAMPLING_64000HZ; break;
        case 88200:
        sample = SAMPLING_88200HZ; break;
        case 96000:
        sample = SAMPLING_96000HZ; break;
        default:
        sample = SAMPLING_16000HZ; break;
    }
    
    au.au.samplerate_lo = (sample >> 1) & 0x7;
    au.au.samplerate_hi = (sample & 0x1);
    
    if (boost::iequals(strAACType, std::string("aac-lc")))
    {
        objt = AUDIO_AAC_LC;
    }
    else if(boost::iequals(strAACType, std::string("aac-main")))
    {
        objt = AUDIO_AAC_MAIN;
    }
    else
    {
        objt = AUDIO_AAC_LC;
    }
    
    au.au.objecttype = objt & 0x1F;
    
    switch (nChannel)
    {
        case 1 : chan = CHANNEL_1CH; break;
        case 2 : chan = CHANNEL_2CH; break;
        case 3 : chan = CHANNEL_3CH; break;
        case 4 : chan = CHANNEL_4CH; break;
        case 5 : chan = CHANNEL_5CH; break;
        case 6 : chan = CHANNEL_6CH; break;
        case 8 : chan = CHANNEL_8CH; break;
        default : chan = CHANNEL_1CH; break;        
    }
    
    au.au.channel = chan & 0xF;
    memcpy (pBuffer, au.buffer, sizeof(AUCONFIG));    
}

};
};
