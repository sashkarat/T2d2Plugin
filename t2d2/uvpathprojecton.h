#ifndef UVPATHPROJECTON_H
#define UVPATHPROJECTON_H

#include "t2d2.h"
#include "clipperlib_f/clipper.hpp"


namespace t2d2 {


class UvPathProjecton
{
    inline int _index(int i, int l)
    {
        return (l + (i % l)) % l;
    }

    class _UvpPoint {
    public:
        float x;
        float y;
        float pos;
        float d;
        _UvpPoint() : x(0), y(0) {}
    };

    _UvpPoint *m_p;
    unsigned int m_len;
    ClipperLib::Path m_clPath;

    void free() {
        if (m_p != nullptr) delete [] m_p;
        m_len = 0;
    }

    void alloc(unsigned int len) {
        m_p = new _UvpPoint [len];
        m_len = len;
    }

    float dist (_UvpPoint &pA, _UvpPoint &pB);

public:

    UvPathProjecton() : m_p(nullptr), m_len(0) {}
    UvPathProjecton(unsigned int len) {alloc(len);}
    ~UvPathProjecton() {free();}


    inline unsigned int len()   { return m_len; }
    inline float x(int index)   { return m_p[_index(index, m_len)].x; }
    inline float y(int index)   { return m_p[_index(index, m_len)].y; }
    inline float pos(int index) { return m_p[_index(index, m_len)].pos; }

    void allocate(unsigned int len) {
        free();
        alloc(len);
    }

    void build(ClipperLib::Path &p);
    int  findNearest(float x, float y);
    bool isInside(float x, float y);
    bool isInside(ClipperLib::IntPoint &p);
    void computeZControlPoints(float bwidth, float holeFactor, ClipperLib::IntPoint &oS, ClipperLib::IntPoint &oE, ClipperLib::IntPoint &wS, ClipperLib::IntPoint &wE);
    bool createZeroPointMiterClip(ClipperLib::Path &res, float bwidth);

    ClipperLib::Path &baseLineClPath() {return m_clPath;}

    static bool isInside(ClipperLib::Path &path, ClipperLib::IntPoint &p);
};

}
#endif // UVPATHPROJECTON_H
