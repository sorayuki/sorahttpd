#ifndef SORA_SOCKET_H_
#define SORA_SOCKET_H_

#include <deque>

namespace Sora
{
    class SocketService;
    class SocketEventsDelegate
    {
    public:
        int socket_fd_;
        int req_events_;

        virtual ~SocketEventsDelegate() {}
        virtual void OnAccept(int socket_fd, SocketService* service) = 0;
        virtual void OnConnected(SocketService* service) = 0;
        virtual void OnRecv(SocketService* service) = 0;
        virtual void OnSend(SocketService* service) = 0;
        virtual void OnDisconnect(SocketService* service) = 0;
        virtual void OnError(SocketService* service) = 0;
    };

    const int SE_ACCEPT = 0x1;
    const int SE_READ = 0x2;
    const int SE_WRITE = 0x4;
    const int SE_ERROR = 0x8;

    class SocketService
    {
    public:
        virtual ~SocketService() {}
        virtual bool RegisterSocketEvent(SocketEventsDelegate* delegate) = 0;
        virtual bool UnregisterSocketEvent(SocketEventsDelegate* delegate) = 0;
        virtual bool RunOnce(int timeoutMs) = 0;
    };

    SocketService* CreateSocketService();
};

#endif
