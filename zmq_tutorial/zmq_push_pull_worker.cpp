#include "zmq_push_pull.hpp"

int main(void)
{
    ZPull worker("tcp://127.0.0.1:5252");
    worker.zconnect();

    ZPush zsink("tcp://127.0.0.1:5253");
    zsink.zconnect();

    int i = 0;

    std::cout << "Workers ready!" << std::endl;

    while (1)
    {
        std::string strRecv = worker.zrecv();
        // 워커는 PULL Client 이고 PUSH 1 : PULL N 의 관계. 1/N 이 된 내용을 받는다.
        zsink.zsend(strRecv.c_str(), strRecv.size());
        // 워커는 PUSH Client 이기도 하다. PUSH N : PULL 1 의 관계이다.
        std::cout << "RECVED SOMETHING AND RESPONSE TO SERVER" << std::endl;
        usleep(1000000); // 워커는 받은 것을 처리한다.
    }

    return 0;
}
