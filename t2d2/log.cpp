#include "log.h"
#include <ostream>

using namespace  t2d2;

LogCallback Log::ms_logCallback = nullptr;

Log::Log() :
    m_logType(ltDebug)
{}

Log::Log(LogType lt) :
    m_logType(lt)
{}

Log::~Log()
{
    std::ostringstream outss;

    switch(m_logType) {
    case ltDebug:
        outss<<"t2d2 [DEBUG]";
        break;
    case ltWarning:
        outss<<"t2d2 [WARNING]";
        break;
    case ltError:
        outss<<"t2d2 [ERROR]";
    }

    outss<<" "<<m_ss.str()<<std::endl;

    if (ms_logCallback != nullptr) {
        ms_logCallback(outss.str().c_str());
    } else {
        if (m_logType == ltDebug)
            std::cout<<outss.str();
        else
            std::cerr<<outss.str();
    }
}

void Log::setCallback(LogCallback lcb)
{
    ms_logCallback = lcb;
}
