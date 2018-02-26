#ifndef SORA_LOG_H_
#define SORA_LOG_H_

#include <sstream>

namespace Sora
{
    class LogOutput : public std::stringstream
    {
    public:
        ~LogOutput();
    };

    using ERROR_LOG = LogOutput;
    using TRACE_LOG = LogOutput;
};

#endif
