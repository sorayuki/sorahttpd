#include "log.h"

#include <iostream>

namespace Sora
{
    LogOutput::~LogOutput()
    {
        std::clog << str();
    }
};
