#ifndef KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGWRAP_H_
#define KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGWRAP_H_

#ifdef __cplusplus
extern "C" {

#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>

#pragma comment (lib, "swscale.lib")
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avutil.lib")
    //#pragma comment (lib, "libavdevice.dll.a")
    //#pragma comment (lib, "libavfilter.dll.a")
#pragma comment (lib, "avformat.lib")
    //#pragma comment (lib, "libpostproc.dll.a")

}
#endif

#include <boost/serialization/singleton.hpp>
#include <boost/thread.hpp>

namespace syncview {
namespace ffmpegwrap {
    
    typedef enum {
        ffmpeg_codec_not_decided = -1,
        ffmpeg_codec_h264 = 0,
        ffmpeg_codec_mpeg4 = 1,
    } fcodec_t;
    
    class ffmpeginitialize : public boost::serialization::singleton<ffmpeginitialize> 
    {
        public:
        ffmpeginitialize(){ 
            av_register_all();
            avcodec_register_all(); 
            avformat_network_init(); 
        };
        virtual ~ffmpeginitialize() { avformat_network_deinit(); }
        void lock() { mmx.lock(); }
        void unlock() { mmx.unlock(); }
        private:
        boost::mutex mmx;
    };
    
}
}

#endif
