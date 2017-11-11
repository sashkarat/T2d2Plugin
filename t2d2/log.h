#ifndef T2D2LOG_H
#define T2D2LOG_H
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "clipperlib_f/clipper.hpp"
#include "t2d2.h"

namespace t2d2 {

typedef void (*LogCallback)(const char *);

enum LogType {
    ltDebug     = 0,
    ltWarning   = 1,
    ltError     = 2
};


class Point;

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

    static void logClipperPath(const std::string &funcName, const std::string &tag, ClipperLib::Path &p);

    Log &operator<<(const ClipperLib::IntPoint &p) {
        *this<<"X"<<p.X<<"Y"<<p.Y;
        return *this;
    }

    Log &operator <<(const t2d2::Point &p);

private:
    static LogCallback ms_logCallback;
    std::ostringstream m_ss;
};

}
#endif // T2D2LOG_H
