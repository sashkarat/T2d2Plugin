#ifndef UVPROJECTION_H
#define UVPROJECTION_H

#include "t2d2.h"

namespace t2d2 {

class t2d2::Point;

class UvProjection
{
    float m_mtx[16];
public:

    UvProjection();


    void setMatrix4x4(float * data);
    void computeUV(t2d2::Point *p);

    static void copy(UvProjection *d, UvProjection *s);
};




}

#endif // UVPROJECTION_H

