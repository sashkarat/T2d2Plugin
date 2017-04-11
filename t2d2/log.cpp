#include "log.h"
#include <ostream>

using namespace  t2d2;

LogCallback Log::ms_logCallback = nullptr;

Log::Log()
{
}

Log::~Log()
{
    std::cout<<m_ss.str()<<std::endl;

    if (ms_logCallback != nullptr) {
        m_ss<<std::endl;
        ms_logCallback(m_ss.str().c_str());
    }
}

void Log::setCallback(LogCallback lcb)
{
    ms_logCallback = lcb;
}
