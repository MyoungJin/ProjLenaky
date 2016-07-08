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

class ZPublisher : public ZMQSock
{
public:

    ZPublisher(const char* addr) : ZMQSock(addr)
    {
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_PUB);
        }

    }

    virtual ~ZPublisher()
    {
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

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        const char* fdelim = "1";
        const char* first = "it sends to first. two can not recv this sentence!\0";

        const char* sdelim = "2";
        const char* second = "it sends to second. one can not recv this sentence!\0";

        while (mbthread)
        {
            zsend(fdelim, 1, true);
            zsend(first, strlen(first));

            zsend(sdelim, 1, true);
            zsend(second, strlen(second));

            std::cout << "RECV TRY" << std::endl;
            std::cout << zrecv() << std::endl;
            std::cout << "RECV TRY" << std::endl;
            std::cout << zrecv() << std::endl;
            std::cout << "RECV TRY" << std::endl;
            std::cout << zrecv() << std::endl;
            std::cout << "RECV TRY" << std::endl;
            std::cout << zrecv() << std::endl;
            std::cout << "RECV TRY" << std::endl;

            usleep(1000 * 1000);
        }
    }

};

class ZSubscriber : public ZMQSock
{
public:

    ZSubscriber(const char* addr) : ZMQSock(addr)
    {
        if (msock == nullptr)
        {
            msock = new zmq::socket_t(*mctx, ZMQ_SUB);
        }
    }

    virtual ~ZSubscriber()
    {
    }

    void setScriberDelim(const char* delim, const int length)
    {
        msock->setsockopt(ZMQ_SUBSCRIBE, delim, length);
        mdelim = std::string(delim, length);
    }

    std::string zrecv()
    {
        zmq::message_t msg;
        msock->recv(&msg);
        return std::string(static_cast<char*>(msg.data()), msg.size());
    }

    void run()
    {
        if (mszAddr == nullptr)
            return ;
        if (strlen(mszAddr) < 6)
            return ;

        while (mbthread)
        {
            std::cout << "MY DELIM IS [" << mdelim << "]  -  MSG : ";
            std::cout << zrecv() << std::endl;

            usleep(1000 * 1000);
        }
    }

private:
    std::string mdelim;
};

int main ()
{
    ZPublisher pub("tcp://*:5252");
    ZSubscriber sub1("tcp://localhost:5252");
    ZSubscriber sub2("tcp://localhost:5252");

    pub.zbind();

    sub1.zconnect();
    sub2.zconnect();

    sub1.setScriberDelim("1", 1);
    sub2.setScriberDelim("2", 1);

    pub.start();
    std::cout << "PUB Server has been started.." << std::endl;

    usleep(1000 * 1000);

    sub1.start();
    std::cout << "SUB1 Start." << std::endl;

    sub2.start();
    std::cout << "SUB2 Start." << std::endl;

    int i = 0;
    std::cout << "< Press any key to exit program. >" << std::endl;
    std::cin >> i;

    std::cout << "SUB1 STOP START" << std::endl;
    sub1.stop();
    std::cout << "SUB2 STOP START" << std::endl;
    sub2.stop();
    std::cout << "PUB  STOP START" << std::endl;
    pub.stop();
    std::cout << "ALL DONE" << std::endl;
    return 0;
}
