#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>

int main(void)
{
    zmq::context_t ctx(1);
#if 1
    zmq::socket_t sock(ctx, ZMQ_REQ); // REQ 타입 소켓, REQUEST 인데 SEND 안하고 RECV 먼저하면 쥬금
#else
    zmq::socket_t sock(ctx, ZMQ_REP); // REP 타입 소켓, REPLY 이니까 RECV 먼저해야 살아
#endif

    zmq::message_t req;
    sock.recv(&req); // RECV를 먼저 해보자

    return 0;
}
