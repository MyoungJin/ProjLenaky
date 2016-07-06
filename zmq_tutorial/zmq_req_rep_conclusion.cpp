#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>

int main(void)
{
    zmq::context_t ctx(1);
#if 1
    zmq::socket_t sock(ctx, ZMQ_REQ); // REQ 타입 소켓
#else
    zmq::socket_t sock(ctx, ZMQ_REP); // REP 타입 소켓
#endif

    zmq::message_t req;
    sock.recv(&req); // RECV를 먼저 해보자

    return 0;
}
