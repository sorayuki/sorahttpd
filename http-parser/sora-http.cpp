#include "sora-http.h"
#include "utils/log.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>

#include <algorithm>
#include <iterator>

namespace Sora
{
    StreamSocket::StreamSocket()
    {
    }

    StreamSocket::~StreamSocket()
    {
    }

    void StreamSocket::OnRecv(SocketService* service)
    {
        int recvSize = recv(socket_fd_, tmpBuf_, BUFSIZE, 0);
        if (recvSize == 0)
            OnDisconnected(service);
        else if (recvSize > 0)
        {
            inputBuffer_.insert(inputBuffer_.end(), &tmpBuf_[0], &tmpBuf_[recvSize]);
            OnDataArrival(service);
        }
        else
        {
            ERROR_LOG() << "recv error: " << strerror(errno);
            OnError(service);
        }
    }

    void StreamSocket::OnSend(SocketService* service)
    {
        if (!outputBuffer_.empty())
        {
            int toSendSize = outputBuffer_.size();
            if (toSendSize > BUFSIZE) toSendSize = BUFSIZE;
            std::copy_n(outputBuffer_.begin(), toSendSize, tmpBuf_);
            int sendSize = send(socket_fd_, tmpBuf_, toSendSize, MSG_NOSIGNAL);
            if (sendSize > 0)
            {
                outputBuffer_.erase(outputBuffer_.begin(), outputBuffer_.begin() + sendSize);

                if (outputBuffer_.empty())
                    OnSendComplete(service);
            }
            else if (sendSize < 0)
            {
                ERROR_LOG() << "send error: " << strerror(errno);
                OnError(service);
            }
        }

        CheckSend(service);
    }

    void StreamSocket::CheckSend(SocketService* service)
    {
        int old_req_events = req_events_;

        if (outputBuffer_.empty())
            req_events_ &= ~SE_WRITE;
        else
            req_events_ |= SE_WRITE;

        if (old_req_events != req_events_)
            service->RegisterSocketEvent(this);
    }

    void StreamSocket::BeginRecv(SocketService* service)
    {
        int old_req_events = req_events_;
        req_events_ |= SE_READ;

        if (old_req_events != req_events_)
            service->RegisterSocketEvent(this);
    }





    static bool MakeSocketNonBlock(int fd)
    {
        long val = 1;
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    HttpServer::HttpServer()
    {
        socket_fd_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_fd_ < 0)
        {
            ERROR_LOG() << "fail to create socket: " << strerror(errno);
            return;
        }

        MakeSocketNonBlock(socket_fd_);

        struct sockaddr_in localEndPoint = {0};
        localEndPoint.sin_family = AF_INET;
        localEndPoint.sin_port = htons(8080);
        localEndPoint.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket_fd_, (struct sockaddr*)&localEndPoint, sizeof(localEndPoint)) != 0)
        {
            ERROR_LOG() << "bind port: " << strerror(errno);
            return;
        }

        if (listen(socket_fd_, 50) != 0)
        {
            ERROR_LOG() << "listen: " << strerror(errno);
            return;
        }

        serv_ = CreateSocketService();
        if (!serv_)
            return;

        req_events_ = SE_ACCEPT;
        serv_->RegisterSocketEvent(this);
    }

    HttpServer::~HttpServer()
    {
        delete serv_;
    }

    void HttpServer::OnAccept(SocketService* service)
    {
        TRACE_LOG() << "ACCEPTED!";

        struct sockaddr_in remoteEndPoint = {0};
        socklen_t addr_len = sizeof(remoteEndPoint);
        int new_socket_fd = accept(socket_fd_, (struct sockaddr*)&remoteEndPoint, &addr_len);

        if (new_socket_fd < 0)
        {
            ERROR_LOG() << "fail to accept: " << strerror(errno);
            return;
        }

        MakeSocketNonBlock(new_socket_fd);
        HttpWorker* worker = new HttpWorker();
        worker->socket_fd_ = new_socket_fd;
        worker->OnConnected(service);
    }

    void HttpServer::Run()
    {
        for(;;)
            serv_->RunOnce(999999);
    }


    HttpWorker::HttpWorker()
    {
    }

    HttpWorker::~HttpWorker()
    {
    }

    void HttpWorker::OnConnected(SocketService* service)
    {
        BeginRecv(service);
    }

    void HttpWorker::OnDisconnected(SocketService* service)
    {
        service->UnregisterSocketEvent(this);
        delete this;
    }

    void HttpWorker::OnError(SocketService* service)
    {
        service->UnregisterSocketEvent(this);
        delete this;
    }

    void HttpWorker::OnDataArrival(SocketService* service)
    {
        TRACE_LOG() << "DATA RECEIVED!";

        bool headerReceived = false;
        do {
            if (inputBuffer_.size() > 4)
            {
                auto rb = inputBuffer_.rbegin();
                if (*rb == '\n' && *(rb + 1) == '\r' && *(rb + 2) == '\n' && *(rb + 3) == '\r')
                {
                    headerReceived = true;
                    break;
                }
            }
        }while(0);

        if (headerReceived)
        {
            TRACE_LOG() << "HEADER RECEIVED!";
            std::string str;
            std::copy(inputBuffer_.begin(), inputBuffer_.end(), std::back_inserter(str));

            std::stringstream ss;
            ss << "HTTP/1.0 200 OK\r\n"
               << "Content-Type: text/plain\r\n"
               << "\r\n"
               << str;
            std::string outputStr = ss.str();
            std::copy(outputStr.begin(), outputStr.end(), std::back_inserter(outputBuffer_));

            CheckSend(service);
        }
    }

    void HttpWorker::OnSendComplete(SocketService* service)
    {
        service->UnregisterSocketEvent(this);
        delete this;
    }
};
