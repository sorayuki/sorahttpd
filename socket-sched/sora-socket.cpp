#include "sora-socket.h"
#include "sorahttpd-config.h"
#include "utils/log.h"
#include <memory>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

namespace Sora
{
    class SocketImpl : public Socket
    {
        friend class SocketService;
    }

    class SocketServiceImpl : public SocketService
    {
        int epoll_fd_;

        void Clean();

    public:
        SocketServiceImpl();
        ~SocketServiceImpl();

        bool Init();
        SocketServiceImpl* GetImpl() override;
        void RunSocketService() override;
    };


    //------------------------------------
    // SocketService

    SocketService::~SocketService()
    {
    }

    int CreateSocketService(SocketService** pService)
    {
        std::unique_ptr<SocketServiceImpl> result( new SocketServiceImpl() );
        if (result->Init())
        {
            *pService = result.release();
            return SOCKSERV_OK;
        }
        else
            return SOCKSERV_ERR;
    }

    SocketServiceImpl::SocketServiceImpl()
        : epoll_fd_(-1)
    {
    }

    SocketServiceImpl::~SocketServiceImpl()
    {
        Clean();
    }
    
    bool SocketServiceImpl::Init()
    {
        epoll_fd_ = epoll_create(EPOLL_MAX_FD_COUNT);
        if (epoll_fd_ < 0)
        {
            ERROR_LOG() << "epoll_create failed. %s" << strerror(errno);
            return false;
        }

        return true;
    }

    void SocketServiceImpl::Clean()
    {
        if (epoll_fd_ >= 0)
        {
            close(epoll_fd_);
            epoll_fd_ = -1;
        }
    }

    SocketServiceImpl* SocketServiceImpl::GetImpl()
    {
        return this;
    }

    //------------------------------------
    // Socket
};