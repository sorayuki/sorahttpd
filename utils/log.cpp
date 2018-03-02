#include "log.h"

#include <iostream>

namespace Sora
{
    LogOutput::~LogOutput()
    {
        *this << std::endl;
        std::clog << str();
    }
};
