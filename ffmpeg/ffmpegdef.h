#ifndef KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGDEF_H_
#define KTSYNCVIEW_TRWRAPFFMPEG_FFMPEGDEF_H_

typedef enum {
    ffmpeg_raw_not_decided = -1,
    ffmpeg_raw_rgb24       = 0,
    ffmpeg_raw_rgb565      = 1, 
    ffmpeg_raw_bgra        = 2,
    ffmpeg_raw_yuv420p     = 3,
} raw_fmt_t;


#endif
