#include "log.h"
#include <fstream>
#include <ostream>
#include <ctime>

using namespace  t2d2;

LogCallback Log::ms_logCallback = nullptr;
std::ofstream Log::m_logFs;

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

    outss<<" "<<m_ss.str();

    std::string outs = outss.str();

    if (ms_logCallback != nullptr) {
        ms_logCallback(outs.c_str());
    } else {
        if (m_logType == ltDebug)
            std::cout<<outs;
        else
            std::cerr<<outs;
    }

    if (m_logFs.is_open()) {
        m_logFs<<outs<<"\r\n"<<std::flush;
    }
}

void Log::setCallback(LogCallback lcb)
{
    ms_logCallback = lcb;
}

void Log::setLogFile(const char *logFileName, bool trunc)
{

    Log()<<__FUNCTION__<<logFileName<<" trunc ="<<trunc;

    if (m_logFs.is_open())
        m_logFs.close();

    std::ios::open_mode om = std::ios::out;

    if (!trunc)
        om |= std::ios::app;

    m_logFs.open(logFileName, om);

    if (!m_logFs.is_open()) {
        Log(ltError)<<__FUNCTION__<<"Unable to open: "<<logFileName;
        return;
    }

    time_t t;
    time(&t);

    m_logFs<<"\r\nLog opened: "<<ctime(&t)<<"\r\n";
}
