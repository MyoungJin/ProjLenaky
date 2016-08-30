#bin/sh
g++ -std=c++11 -o hlssegment hlssegment.cpp -D__STDC_CONSTANT_MACROS -lpthread -lavdevice -lavfilter -lavformat -lavcodec -lswresample -lswscale -lavutil -lx264 -lpostproc -lz -ldl
