#ifndef T2D2LOG_H
#define T2D2LOG_H
#include <iostream>
#include <sstream>
#include <fstream>

namespace t2d2 {

typedef void (*LogCallback)(const char *);

enum LogType {
    ltDebug     = 0,
    ltWarning   = 1,
    ltError     = 2
};


class Log
{
    LogType m_logType;

    static std::ofstream m_logFs;

public:

    Log ();
    Log (LogType lt);

    ~Log();

    template <class T> Log &operator<<(const T &v) {
        m_ss<<v<<" ";
        return *this;
    }

    static void setCallback(LogCallback lcb);
#ifndef ANDROID
    static void setLogFile(const char *logFileName, bool trunc);
#endif

private:
    static LogCallback ms_logCallback;
    std::ostringstream m_ss;
};

}
#endif // T2D2LOG_H
