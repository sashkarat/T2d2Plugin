#ifndef T2D2LOG_H
#define T2D2LOG_H
#include <iostream>
#include <sstream>

namespace t2d2 {

typedef void (*LogCallback)(const char *);

class Log
{
public:

    Log();

    ~Log();

    static void setCallback(LogCallback lcb);

    template <class T> Log &operator<<(const T &v) {
        m_ss<<v<<" ";
        return *this;
    }

private:
    static LogCallback ms_logCallback;
    std::ostringstream m_ss;

};

}
#endif // T2D2LOG_H
