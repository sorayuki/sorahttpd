#include "sora-socket.h"
#include "sorahttpd-config.h"
#include "utils/log.h"
#include <memory>
#include <vector>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

namespace Sora
{
    class SocketServiceImpl : public SocketService
    {
        int epoll_fd_;
        std::vector<struct epoll_event> epoll_result_events_;

        void Clean();

    public:
        SocketServiceImpl();
        ~SocketServiceImpl();

        bool Init();

        bool RegisterSocketEvent(SocketEventsDelegate* delegate) override;
        bool UnregisterSocketEvent(SocketEventsDelegate* delegate) override;
        bool RunOnce(int timeoutMs) override;
    };


    //------------------------------------
    // SocketService

    SocketService* CreateSocketService()
    {
        std::unique_ptr<SocketServiceImpl> result( new SocketServiceImpl() );
        if (result->Init())
            return result.release();
        else
            return nullptr;
    }

    SocketServiceImpl::SocketServiceImpl()
        : epoll_fd_(-1)
        , epoll_result_events_(EPOLL_MAX_FD_COUNT)
    {
    }

    SocketServiceImpl::~SocketServiceImpl()
    {
        Clean();
    }
    
    bool SocketServiceImpl::Init()
    {
        Clean();
        
        epoll_fd_ = epoll_create(EPOLL_MAX_FD_COUNT);
        if (epoll_fd_ < 0)
        {
            ERROR_LOG() << "epoll_create failed. " << strerror(errno);
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

    bool SocketServiceImpl::RegisterSocketEvent(SocketEventsDelegate* delegate)
    {
        struct epoll_event ev{};
        if (delegate->req_events_ & SE_READ) ev.events |= EPOLLIN;
        if (delegate->req_events_ & SE_WRITE) ev.events |= EPOLLOUT;
        if (delegate->req_events_ & SE_ERROR) ev.events |= EPOLLERR;
        ev.data.ptr = delegate;

        int action = EPOLL_CTL_ADD;
        for(;;)
        {
            int ret = epoll_ctl(epoll_fd_, action, delegate->socket_fd_, &ev);
            if (ret == 0)
                break;
            if (errno == EINTR)
                continue;
            if (errno == EEXIST && action == EPOLL_CTL_ADD)
            {
                action = EPOLL_CTL_MOD;
                continue;
            }
            
            ERROR_LOG() << "epoll_ctl " << (action == EPOLL_CTL_ADD ? "add" : "mod") << " failed. " << strerror(errno);
            return false;
        }

        return true;
    }

    bool SocketServiceImpl::UnregisterSocketEvent(SocketEventsDelegate* delegate)
    {
        for(;;)
        {
            int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, delegate->socket_fd_, 0);
            if (ret == 0)
                break;
            if (errno == EINTR)
                continue;
            if (errno == ENOENT)
                break;

            ERROR_LOG() << "epoll_ctl del failed. " << strerror(errno);
            return false;
        }
        return true;
    }

    bool SocketServiceImpl::RunOnce(int timeoutMs)
    {
        epoll_wait(epoll_fd_, epoll_result_events_.data(), EPOLL_MAX_FD_COUNT, timeoutMs);
        return true;
    }
};
