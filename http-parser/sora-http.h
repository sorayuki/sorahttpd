#ifndef SORA_HTTP_PROTO_H_
#define SORA_HTTP_PROTO_H_

#include "socket-sched/sora-socket.h"
#include <unordered_map>

namespace Sora
{
    class StreamSocket
        : public SocketScaffold
    {
    protected:
        enum { BUFSIZE = 1024 };
        char tmpBuf_[BUFSIZE];
        void OnRecv(SocketService* service) override;
        void OnSend(SocketService* service) override;

    public:
        virtual void OnConnected(SocketService* service) = 0;
        virtual void OnDisconnected(SocketService* service) = 0;
        virtual void OnError(SocketService* service) = 0;

        virtual void OnDataArrival(SocketService* service) = 0;
        virtual void OnSendComplete(SocketService* service) = 0;

    public:
        std::deque<char> inputBuffer_;
        std::deque<char> outputBuffer_;

        void CheckSend(SocketService* service);
        void BeginRecv(SocketService* service);

        StreamSocket();
        ~StreamSocket();
    };


    class HttpServer
        : public SocketScaffold
    {
    protected:
        SocketService* serv_;

    protected:
        void OnAccept(SocketService* service) override;

    public:
        HttpServer();
        ~HttpServer();

        void Run();
    };


    class HttpWorker
        : public StreamSocket
    {
    public:
        void OnConnected(SocketService* service) override;
        void OnDisconnected(SocketService* service) override;
        void OnError(SocketService* service) override;
    
        void OnDataArrival(SocketService* service) override;
        void OnSendComplete(SocketService* service) override;

    protected:
        std::unordered_map<std::string, std::string> httpHeaders_;

    public:
        HttpWorker();
        ~HttpWorker();
    };
};

#endif
