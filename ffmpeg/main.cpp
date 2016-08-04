#include <iostream>
#include <fstream>
#include <chrono>

#include "ffmpegdef.h"
#include "ffmpegenc.h"

#define HD_YUV_SIZE (1280 * 720 * 3) / 2

using namespace syncview;
using namespace ffmpegenc;

int main(void)
{
    std::ifstream file(".\\Camera_Raw_HD_Dump.yuv", 
                        std::ios::in | std::ios::binary);

    char* buf = new char[HD_YUV_SIZE];
    const int nSize = HD_YUV_SIZE;

#define NVENC 1

#if NVENC
    syncview::ffmpegenc::FEncoder fe(std::string("h264_nvenc"), 
            ffmpeg_prof_baseline, 1280, 720, 
            ffmpeg_raw_yuv420p,
        1024 * 1024, 30);
#else
    syncview::ffmpegenc::FEncoder fe(std::string("h264"),
        ffmpeg_prof_baseline, 1280, 720,
        ffmpeg_raw_yuv420p,
        1024 * 1024, 30);
#endif

    unsigned int nEncodedSize = 0;
    bool isIDR = false;
    unsigned char* pEncodedBuffer = NULL;

    int s = 0;

    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();

    int idx = 0;
    while (!file.eof())
    {
        file.read(buf, nSize);
        int nRet = fe.Encode((unsigned char*)buf, nSize, &pEncodedBuffer, nEncodedSize, isIDR);

        if (nEncodedSize > 0)
            std::cout << "Encode : Ret {" << nRet << "}  "
                << "Size : " << nEncodedSize << "  Is IDR : " << (isIDR ? "yes" : "no")
                << std::endl;

        if (file.eof())
        {
            if (idx++ < 10)
            {
                file.clear();
                file.seekg(0, std::ios::beg);
            }
        }

        if (isIDR) s++;
    }

    while (true)
    {
        nEncodedSize = 0;
        int nRet = fe.Encode(NULL, nSize, &pEncodedBuffer, nEncodedSize, isIDR);

        if (nEncodedSize > 0)
        {
            std::cout << "Reamining Encode : Ret {" << nRet << "}  "
                << "Size : " << nEncodedSize << "  Is IDR : " << (isIDR ? "yes" : "no")
                << std::endl;
            if (isIDR) s++;
        }
        else break;        
    }

    file.close();
    delete [] buf;

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - t;
    std::cout << "done, elapsed -> " << sec.count() << "  idr = " << s <<  std::endl;
    return 0;
}