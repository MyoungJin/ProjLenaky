#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>

class ZReqRep // request and reply
{
public:

    ZReqRep(const char* addr)
    {
        if (addr != NULL)
        {
            mctx = new zmq::context_t(1);
            mszAddr = new char[strlen(addr) + 1];
            snprintf(mszAddr, strlen(addr) + 1, "%s", addr);
        }
    }

    virtual ~ZReqRep()
    {
        if (msock != nullptr)
            delete msock;
        if (mctx != nullptr)
            delete mctx;
        if (mszAddr != nullptr)
            delete [] mszAddr;
    }

    virtual void run() = 0;

    void setRecvTimer(int milli)
    {
        if (msock != nullptr)
            msock->setsockopt(ZMQ_RCVTIMEO, &milli, sizeof(int));
    }

    void start()
    {
        if (mbthread != false)
            return ;

        mbthread = true;
        mhthread = std::thread(std::bind(&ZReqRep::run, this));
    }

    virtual void stop()
    {
        if (mbthread == false)
            return ;

        mbthread = false;
        if (mhthread.joinable())
            mhthread.join();
    }

protected:
    char* mszAddr{nullptr};
    zmq::context_t* mctx{nullptr};
    zmq::socket_t* msock{nullptr};
    bool mbthread{false};
    std::thread mhthread;
};

class ZCli : public ZReqRep
{
public:

    ZCli(const char* addr) : ZReqRep(addr)
    {
        std::cout << "zcli const" << std::endl;
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_REQ);
            std::cout << "socket create" << std::endl;;
        }

    }

    virtual ~ZCli()
    {
    }

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        std::cout << "CLIENT :: connecting to \"world\" server" << std::endl;
        msock->connect(mszAddr);

        while (mbthread)
        {
            zmq::message_t req(5);
            memcpy(req.data(), "hello", 5);
            msock->send(req);
            std::cout << "CLIENT :: send \"hello\" to server" << std::endl;

            zmq::message_t rep;
            msock->recv(&rep);
            std::cout << "CLIENT :: recv from server -> " << (const char*)rep.data() << std::endl;

            zmq::message_t rep2;
            msock->recv(&rep2);
            std::cout << "CLIENT :: recv AGAIN from server -> " << (const char*)rep2.data() << std::endl;
            usleep(1000 * 1000);
        }
    }

};

class ZServ : public ZReqRep
{
public:

    ZServ(const char* addr) : ZReqRep(addr)
    {
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_REP);
        }
    }

    virtual ~ZServ()
    {
    }

    void stop()
    {
        if (mbthread == false)
            return ;

        mbthread = false;
        if (mhthread.joinable())
            mhthread.join();
    }

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        msock->bind(mszAddr);

        std::cout << "SERVER :: \"world\" server is online" << std::endl;

        while (mbthread)
        {
            zmq::message_t req;
            if (msock->recv(&req) == 0)
            {
                usleep(1000 * 10);
                continue ;
            }

            std::cout << "SERVER :: recv from client -> " << (const char*)req.data() <<  std::endl;
            zmq::message_t rep(6);
            memset(rep.data(), 0x00, 6);
            memcpy(rep.data(), "world\0", 6);
            msock->send(rep, ZMQ_SNDMORE);
            std::cout << "SERVER :: send \"world\" to client" << std::endl;

            zmq::message_t rep2(6);
            memset(rep2.data(), 0x00, 6);
            memcpy(rep2.data(), "idiot\0", 6);
            msock->send(rep2);
            std::cout << "SERVER :: send \"idiot\" to client" << std::endl;
            usleep(1000 * 1000);
        }
    }
};

int main ()
{
    ZServ s("tcp://*:5252");
    ZCli c("tcp://localhost:5252");

    s.setRecvTimer(300);

    std::cout << "Hello and World Co-working Start... " << std::endl;
    s.start();
    std::cout << "World Server has been started... " << std::endl;
    usleep(1000 * 1000);

    c.start();
    std::cout << "Hello Client has been started... " << std::endl;

    int i = 0;
    std::cout << "< Press any key to exit program. >" << std::endl;
    std::cin >> i;

    std::cout << "Client join stated." << std::endl;
    c.stop();
    std::cout << "Client join done. Server join started." << std::endl;
    s.stop();
    std::cout << "Server join done." << std::endl;
    return 0;
}
