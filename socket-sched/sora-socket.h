#ifndef SORA_SOCKET_H_
#define SORA_SOCKET_H_

#include <deque>

namespace Sora
{
    class SocketService;
    class SocketScaffold
    {
    public:
        int socket_fd_;
        int req_events_;

        SocketScaffold();
        virtual ~SocketScaffold();
        virtual void OnAccept(SocketService* service) {}
        virtual void OnConnected(SocketService* service) {}
        virtual void OnRecv(SocketService* service) {}
        virtual void OnSend(SocketService* service) {}
        virtual void OnError(SocketService* service) {}
    };

    const int SE_ACCEPT = 0x1;
    const int SE_READ = 0x2;
    const int SE_WRITE = 0x4;
    const int SE_ERROR = 0x8;

    class SocketService
    {
    public:
        virtual ~SocketService() {}
        virtual bool RegisterSocketEvent(SocketScaffold* socketScaffold) = 0;
        virtual bool UnregisterSocketEvent(SocketScaffold* socketScaffold) = 0;
        virtual bool RunOnce(int timeoutMs) = 0;
    };

    SocketService* CreateSocketService();
};

#endif
