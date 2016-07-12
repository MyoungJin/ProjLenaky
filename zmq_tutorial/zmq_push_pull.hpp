#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>

class ZMQSock
{
public:
    ZMQSock(const char* addr)
    {
        if (addr != NULL)
        {
            mctx = new zmq::context_t(1);
            mszAddr = new char[strlen(addr) + 1];
            snprintf(mszAddr, strlen(addr) + 1, "%s", addr);
        }
    }

    virtual ~ZMQSock()
    {
        if (msock != nullptr)
            delete msock;
        if (mctx != nullptr)
            delete mctx;
        if (mszAddr != nullptr)
            delete [] mszAddr;
    }

    int zbind()
    {
        if (msock != nullptr)
            msock->bind(mszAddr);
        else return -1;
        return 0;
    }

    int zconnect()
    {
        if (msock != nullptr)
            msock->connect(mszAddr);
        else return -1;
            return 0;
    }

    bool zsend(const char* data, const unsigned int length, bool sendmore=false)
    {
        zmq::message_t msg(length);
        memcpy(msg.data(), data, length);
        if (sendmore)
            return msock->send(msg, ZMQ_SNDMORE);
        return msock->send(msg);
    }

    std::string zrecv()
    {
        zmq::message_t msg;
        msock->recv(&msg);
        return std::string(static_cast<char*>(msg.data()), msg.size());
    }

    void start()
    {
        if (mbthread != false)
            return ;

        mbthread = true;
        mhthread = std::thread(std::bind(&ZMQSock::run, this));
    }

    virtual void stop()
    {
        if (mbthread == false)
            return ;

        mbthread = false;
        if (mhthread.joinable())
            mhthread.join();
    }

    virtual void run() = 0;

protected:
    char* mszAddr{nullptr};
    zmq::context_t* mctx{nullptr};
    zmq::socket_t* msock{nullptr};
    bool mbthread{false};
    std::thread mhthread;
};

class ZPush : public ZMQSock
{
public:

    ZPush(const char* addr) : ZMQSock(addr)
    {
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_PUSH);
        }

    }

    virtual ~ZPush()
    {
    }

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        while (mbthread)
        {

        }
    }

};

class ZPull : public ZMQSock
{
public:

    ZPull(const char* addr) : ZMQSock(addr)
    {
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_PULL);
        }
    }

    virtual ~ZPull()
    {
    }

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        while (mbthread)
        {

        }
    }

};
