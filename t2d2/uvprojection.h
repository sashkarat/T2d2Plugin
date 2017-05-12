#ifndef UVPROJECTION_H
#define UVPROJECTION_H

#include "t2d2.h"

namespace t2d2 {

class t2d2::Point;

class UvProjection
{
    struct _P {
        float x;
        float y;
        _P() : x(0), y(0) {}
        _P(float _x, float _y) : x(_x), y(_y) {}
        inline void set(float _x, float _y) {x = _x; y = _y;}
        inline void set(float *data) {x = data[0]; y = data[1];}
    };

    float m_mtx[16];
public:

    UvProjection();


    void setMatrix4x4(float * data);
    void computeUV(t2d2::Point *p, float &u, float &v);
};




}

#endif // UVPROJECTION_H

