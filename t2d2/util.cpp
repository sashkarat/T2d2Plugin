#include <stdlib.h>
#include <cstring>
#include <algorithm>
#include "util.h"

#include "log.h"

#ifndef LESS_OR_EQ
#define LESS_OR_EQ(a,b) (almostEqual2sComplement(a, b, 1) || a < b)
#endif

#ifndef MORE_OR_EQ
#define MORE_OR_EQ(a,b) (almostEqual2sComplement(a, b, 1) || a > b)
#endif


extern float orient2d(float* pa, float* pb, float* pc);

using namespace t2d2;


bool t2d2::almostEqual2sComplement(float a, float b, int maxUlps)

{
    // Make sure maxUlps is non-negative and small enough that the

    // default NAN won't compare as equal to anything.

    maxUlps = (maxUlps > 0) ? maxUlps : 1;
    maxUlps = (maxUlps < 4194304) ? maxUlps : 4194304;

    int aInt = *(int*)&a;

    // Make aInt lexicographically ordered as a twos-complement int

    if (aInt < 0)

        aInt = 0x80000000 - aInt;

    // Make bInt lexicographically ordered as a twos-complement int

    int bInt = *(int*)&b;

    if (bInt < 0)

        bInt = 0x80000000 - bInt;

    int intDiff = abs(aInt - bInt);

    if (intDiff <= maxUlps)

        return true;

    return false;

}


struct _P {
    float *data;
    bool clrMem;
    _P(float x, float y) : data(new float[2]), clrMem(true) { data[0] = x; data[1] = y;}
    _P(float *_data) : data(_data), clrMem(false) {}

    ~_P() { if (clrMem) delete [] data; }
    inline float x() {return data[0];}
    inline float y() {return data[1];}

    friend inline bool operator == (_P &a, _P &b) {
        if ( t2d2::almostEqual2sComplement(a.data[0], b.data[0], 1)
                && t2d2::almostEqual2sComplement(a.data[1], b.data[1], 1))
            return true;
        return false;
    }

    inline static float dot(_P &a, _P &b) {
        return a.data[0] * b.data[0] + a.data[1] * b.data[1];
    }

    inline float len2() {
        return data[0] * data[0] + data[1] * data[1];
    }

    static friend Log &operator <<(Log &l, _P &p) {
        l<<"["<<p.data[0]<<p.data[1]<<"]";
        return l;
    }

};


class _BoundingBox {
public:
    float xmin;
    float xmax;
    float ymin;
    float ymax;

    inline _BoundingBox(_P &p1, _P &p2) {
        xmin = std::min(p1.x(), p2.x());
        xmax = std::max(p1.x(), p2.x());
        ymin = std::min(p1.y(), p2.y());
        ymax = std::max(p1.y(), p2.y());
    }

    inline bool contains(_P &p) {
        return
                LESS_OR_EQ(xmin, p.x()) &&
                LESS_OR_EQ(p.x(), xmax) &&
                LESS_OR_EQ(ymin, p.y()) &&
                LESS_OR_EQ(p.y(), ymax);

    }
};

//inline float triArea2 (_P &p0, _P &p1, _P &p2)
//{
//    return (p1.x() - p0.x()) * (p2.y() - p0.y()) - (p2.x() - p0.x()) * (p1.y() - p0.y());
//}


inline int orientation(_P &p0, _P &p1, _P &p2)
{
    float v = orient2d(p0.data, p1.data, p2.data);

//    float v = triArea2(p0, p1, p2);

//    Log(ltDebug)<<__FUNCTION__<<p0<<p1<<p2<<"v:"<<v;

    return almostEqual2sComplement(v, 0, 1) ? 0 : ((v > 0) ? 1 : 2);
}

bool isIntersection(_P &pa, _P &pb, _P &pc, _P &pd)
{

//    Log(ltDebug)<<__FUNCTION__<<"pa"<<pa<<"pb"<<pb<<"pc"<<pc<<"pd"<<pd;

    bool co = true;

    _BoundingBox bba(pa, pb);
    _BoundingBox bbc(pc, pd);

    int abc = orientation (pa, pb, pc);

//    Log(ltDebug)<<"orientation abc"<<abc;


    if (abc == 0 ) {
        if (bba.contains(pc))
            return true;
        co = false;
    }

    int abd = orientation (pa, pb, pd);

//    Log(ltDebug)<<"orientation abd"<<abd;

    if (abd == 0 ) {
        if (bba.contains(pd))
            return true;
        co = false;
    }

    int cda = orientation (pc, pd, pa);

//    Log(ltDebug)<<"orientation cda"<<cda;

    if (cda == 0 ) {
        if (bbc.contains(pa))
            return true;
        co = false;
    }

    int cdb = orientation (pc, pd, pb);

//    Log(ltDebug)<<"orientation cdb"<<cdb;

    if (cdb == 0 ) {
        if (bbc.contains(pb))
            return true;
        co = false;
    }

    if (co && abc != abd && cda != cdb)
        return true;

    return false;
}


bool projectPointOnLine(_P &a, _P &b, _P &c, _P &res)
{
  // get dot product of e1, e2
  _P e1 (b.x() - a.x(), b.y() - a.y());
  _P e2 (c.x() - a.x(), c.y() - a.y());

  float valDp = _P::dot(e1, e2);
  // get squared length of e1
  float len2 = e1.len2();

  res.data[0] = (a.x() + (valDp * e1.x()) / len2);
  res.data[1] = (a.y() + (valDp * e1.y()) / len2);

  _BoundingBox bb(a, b);

  return bb.contains(res);
}

bool t2d2::intersects(float *a, float *b, float *c, float *d)
{
    return isIntersection(_P(a), _P(b), _P(c), _P(d));
}

bool t2d2::pointToSegmentProjection(float *a, float *b, float *c, float *res)
{
    return projectPointOnLine(_P(a), _P(b), _P(c), _P(res));
}


bool t2d2::pointOnSegment(float *a, float *b, float *c)
{
    _P _a(a);
    _P _b(b);
    _P _c(c);

    if (_a ==_c || _b == _c) {
        return true;
    }

    _P res(0,0);

    if (!projectPointOnLine(_a, _b, _c, res))
        return false;

    res.data[0] = _c.x() - res.x();
    res.data[1] = _c.y() - res.y();

    if (almostEqual2sComplement(res.len2(), 0.0f, 1))
        return true;
    return false;
}

int _index(int x, int l)
{
    return (l + (x % l)) % l;
}


bool t2d2::pointInPolygon(float *poly, int length, int stride, float *point)
{
    float *pC = point;
    float pD[2];
    pD[1] = pC[1];

    int c = 0;

    for(int i = 0; i < length; i++)
    {
        int j = _index(i+1, length);

        float *pA = poly + i * stride;
        float *pB = poly + j * stride;

//        Log(ltDebug)<<"i"<<i<<"pA:"<<pA[0]<<pA[1]<<"  pB:"<<pB[0]<<pB[1];

        if (pointOnSegment(pA, pB, pC)) {
//            Log(ltDebug)<<"point on segment";
            return true;
        }

        pD[0] = std::max(pA[0], pB[0]);

        if (pD[0] < pC[0]) {
//            Log(ltDebug)<<"pD[0]"<<pD[0]<<" is less";
            continue;
        }

        pD[0] += 10;


//        Log(ltDebug)<<"Ray:"<<pC[0]<<"-"<<pD[0]<<"   /"<<pD[1];

        if (intersects(pA, pB, pC, pD)) {
            c++;
//            Log(ltDebug)<<"intersection. c:"<<c;
        } else {
//            Log(ltDebug)<<"No intersection:"<<"pA["<<pA[0]<<pA[1]<<"]  pB[:]"<<pB[0]<<pB[1]<<"]";
            continue;
        }

        if (pointOnSegment(pC, pD, pA)) {

            int oCAB, oCAPrA;
            oCAB = oCAPrA = 0;

            int in = _index(i, length);
            int ib = _index (i - 1, length);

            while (oCAB == 0) {
                in = _index (in, length);
                int jn = _index (in + 1, length);

                float *_pA = poly + in * stride;
                float *_pB = poly + jn * stride;
                oCAB = orientation(_P(pC), _P(_pA), _P(_pB));
                in++;
            }

            while (oCAPrA == 0) {

                ib = _index (ib, length);
                int jb = _index (ib + 1, length);

                float *_pA = poly + ib * stride;
                float *_pB = poly + jb * stride;
                oCAPrA = orientation(_P(pC), _P(_pB), _P(_pA));
                ib--;
            }

            if (oCAB == oCAPrA) {
                c+=2;
            } else {
                c++;
            }
//            Log(ltDebug)<<"pA on ray. oCAB:"<<oCAB<<"oCAPrA"<<oCAPrA<<"c:"<<c;

        }
    }
    return (c%2) == 1;
}

int t2d2::contourEdgeSelfIntersection(float *poly, int length, int stride, int index)
{
    int j_index = _index (index+1, length);

    float *pA = poly + index * stride;
    float *pB = poly + j_index * stride;

    int is = _index(index + 2, length);
    int ie = _index (index -1, length);

    int i = is;

    while ( i != ie)
    {
        int j = _index(i + 1, length);
        float *pC = poly + i * stride;
        float *pD = poly + j * stride;

        if ( intersects(pA, pB, pC, pD) )
            return i;
        i = j;
    }

    return -1;
}

int t2d2::segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC)
{
    float *pA = segment;
    float *pB = segment + strideS;

    for (int i = 0; i < length; i++) {
        float *pC = contour + i * strideC;
        float *pD = contour + _index (i + 1, length) * strideC;

        if (intersects(pA, pB, pC, pD))
            return i;
    }

    return -1;
}

int t2d2::findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out)
{
    float d2;
    int index = -1;
    _P outP(0,0);
    _P delta(0,0);

    for(int i = 0; i < length; i++) {
        float *pA = contour + i * stride;
        float *pB = contour + _index (i + 1, length) * stride;

        _P a(pA);
        _P b(pB);
        _P c(point);
        _P res(0,0);

        if (!projectPointOnLine(a, b, c, res))
            continue;
        delta.data[0] = c.x() - res.x();
        delta.data[1] = c.y() - res.y();

        float rl2 = delta.len2();

        if (index < 0) {
            index = i;
            d2 = rl2;
            outP.data[0] = res.x();
            outP.data[1] = res.y();
        } else {
            if (rl2 < d2) {
                index = i;
                d2 = rl2;
                outP.data[0] = res.x();
                outP.data[1] = res.y();
            }
        }
    }

    if (out != nullptr && index >= 0) {
        out[0] = outP.x();
        out[1] = outP.y();
    }

    return index;
}

void t2d2::getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax)
{
    float xmin = 1e38f;
    float xmax = -1e37f;
    float ymin = 1e38f;
    float ymax = -1e37f;

    for(int i = 0; i < length; i++) {
        _P p(contour);
        if (p.x() < xmin)
            xmin = p.x();
        if (p.x() > xmax)
            xmax = p.x();
        if (p.y() < ymin)
            ymin = p.y();
        if (p.y() > ymax)
            ymax = p.y();
        contour += stride;
    }

    outMin[0] = xmin;
    outMax[0] = xmax;
    outMin[1] = ymin;
    outMax[1] = ymax;
}
