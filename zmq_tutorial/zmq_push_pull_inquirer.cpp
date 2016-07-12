#include "zmq_push_pull.hpp"

int main(void)
{
    ZPush zinq("tcp://127.0.0.1:5252");
    zinq.zbind(); // PUSH 서버를 하나 둔다. 이건 워커들에게 PUSH 할 것들.

    ZPull zsink("tcp://127.0.0.1:5253");
    zsink.zbind(); // PULL 서버를 하나 둔다. 이건 워커들로부터 처리 내용을 받을 PULL 들.

    int i = 0;

    std::cout << "Ready. Let workers know operation will be started" << std::endl;
    std::cout << "Press Any Key to Start." << std::endl;
    std::cin >> i;

    const char* sleep100ms = "sleep 100 ms";

    std::cout << "SEND TO WORKER : " << sleep100ms << std::endl;

    for (int i = 0 ; i < 10 ; i++) //  보낸만큼
        zinq.zsend(sleep100ms, strlen(sleep100ms)); // 워커에게 메시지를 보낸다.
        // ZMQ가 알아서 PULL Client 들에게 Evenly 하게 메세지를 보낼 것이다.
        // WORKER 가 1개이면 10개를 전부 다,
        // WORKER 가 N개이면 10/N 개씩 보낸다.

    for (int i = 0 ; i < 10 ; i++) // 응답이 오면 나가는 것으로.
        std::cout << "RECEIVE : " << zsink.zrecv() << std::endl;
        // Worker PUSH Client 들로부터 메시지를 받을 것이다.
        // Worker 가 여러개이면 병렬로 처리될 것이다.

    std::cout << "All done." << std::endl;

    return 0;
}
