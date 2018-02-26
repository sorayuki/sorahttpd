cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

project (sorahttpd)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (EPOLL_MAX_FD_COUNT 1024)

configure_file ("${PROJECT_SOURCE_DIR}/sorahttpd-config.h.in" "${PROJECT_SOURCE_DIR}/sorahttpd-config.h")

include_directories( ${PROJECT_SOURCE_DIR} )

set (SOCKET_SCHED_SRCS
    socket-sched/sora-socket.cpp
)

set (UTILS_SRCS
    utils/log.cpp
)

add_executable(sorahttpd
    ${SOCKET_SCHED_SRCS}
    ${UTILS_SRCS}
)
