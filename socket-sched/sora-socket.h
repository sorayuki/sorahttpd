#ifndef SORA_SOCKET_H_
#define SORA_SOCKET_H_

#include <deque>

namespace Sora
{
    enum{
        SOCKSERV_OK = 0,
        SOCKSERV_ERR = 1
    };

    class SocketServiceImpl;
    class SocketService
    {
    public:
        virtual ~SocketService() = 0;

        virtual SocketServiceImpl* GetImpl() = 0;
        virtual void RunSocketService() = 0;
    };

    int CreateSocketService(SocketService** pService);

    class Acceptor
    {
    public:
        
    };

    Acceptor* CreateAcceptor(SocketService* service, const char* ip, int port);

    class Socket
    {
    public:

    };

    Socket* CreateSocketTo(SocketService* service, const char* dest, int port);
};

#endif
