#ifndef T2D2LOG_H
#define T2D2LOG_H
#include <iostream>

namespace t2d2 {

class Log
{
public:
    Log();
    ~Log();
    static Log log();
private:
    std::ostringstream m_ss;
};

}
#endif // T2D2LOG_H
