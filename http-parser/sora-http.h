#ifndef SORA_HTTP_PROTO_H_
#define SORA_HTTP_PROTO_H_

#include "socket-sched/sora-socket.h"

namespace Sora
{
    class SocketAcceptor
        : public SocketScaffold
    {
    protected:
        void OnAccept(SocketService* service) override;

    public:
        SocketAcceptor();
        ~SocketAcceptor();
    };


    class StreamSocket
        : public SocketScaffold
    {
    protected:
        void OnConnected(SocketService* service) override;
        void OnRecv(SocketService* service) override;
        void OnSend(SocketService* service) override;
        void OnError(SocketService* service) override;

    protected:
        virtual bool OnDataArrival() = 0;

    protected:
        std::deque<char> inputBuffer_;
        std::deque<char> outputBuffer_;

    public:
        StreamSocket();
        ~StreamSocket();
    };


    class HttpSocket
        : public StreamSocket
    {
    protected:
        bool OnDataArrival() override;

    public:
        HttpSocket();
        ~HttpSocket();
    };
};

#endif
