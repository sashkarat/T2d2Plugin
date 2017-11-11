#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <algorithm>

#include "util.h"
#include "contour.h"


#include "log.h"


using namespace t2d2;
//using namespace t2d2::util;


bool util::almostEqual2sComplement(float a, float b, int maxUlps)

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


struct _Point {
    float sdata[2];
    float *data;

    _Point(p2t::Point *p) {data = sdata; data[0] = p->x; data[1] = p->y;}
    _Point(float x, float y) {data = sdata; data[0] = x; data[1] = y;}
    _Point(float * _data) { data = _data;}

    inline const float x() const {return data[0];}
    inline const float y() const {return data[1];}
    inline void setX(float x) {data[0] = x;}
    inline void setY(float y) {data[1] = y;}

    friend inline bool operator == (_Point &a, _Point &b) {
        if ( t2d2::util::almostEqual2sComplement(a.x(), b.x(), 1)
                && t2d2::util::almostEqual2sComplement(a.y(), b.y(), 1))
            return true;
        return false;
    }

    inline static float dot(_Point &a, _Point &b) {
        return a.x() * b.x() + a.y() * b.y();
    }

    inline float len2() {
        return x() * x() + y() * y();
    }

//    static friend Log &operator <<(Log &l, _Point &p) {
//        l<<"["<<p.x()<<p.y()<<"]";
//        return l;
//    }
};


class _BoundingBox {
public:
    float xmin;
    float xmax;
    float ymin;
    float ymax;

    inline _BoundingBox(_Point &p1, _Point &p2) {
        xmin = std::min(p1.x(), p2.x());
        xmax = std::max(p1.x(), p2.x());
        ymin = std::min(p1.y(), p2.y());
        ymax = std::max(p1.y(), p2.y());
    }

    inline bool contains(_Point &p) {
        return
                t2d2::util::lessOrEq(xmin, p.x()) &&
                t2d2::util::lessOrEq(p.x(), xmax) &&
                t2d2::util::lessOrEq(ymin, p.y()) &&
                t2d2::util::lessOrEq(p.y(), ymax);
    }
};

inline int orientation(const _Point &p0, const _Point &p1, const _Point &p2)
{
    float v = t2d2::util::orient(p0.x(), p0.y(), p1.x(), p1.y(), p2.x(), p2.y());
    return t2d2::util::almostEqual2sComplement(v, 0, 1) ? 0 : ((v > 0) ? 1 : 2);
}

bool isIntersection(_Point &pa, _Point &pb, _Point &pc, _Point &pd)
{

    bool co = true;

    _BoundingBox bba(pa, pb);
    _BoundingBox bbc(pc, pd);

    int abc = orientation (pa, pb, pc);

    if (abc == 0 ) {
        if (bba.contains(pc))
            return true;
        co = false;
    }

    int abd = orientation (pa, pb, pd);

    if (abd == 0 ) {
        if (bba.contains(pd))
            return true;
        co = false;
    }

    int cda = orientation (pc, pd, pa);

    if (cda == 0 ) {
        if (bbc.contains(pa))
            return true;
        co = false;
    }

    int cdb = orientation (pc, pd, pb);

    if (cdb == 0 ) {
        if (bbc.contains(pb))
            return true;
        co = false;
    }

    if (co && abc != abd && cda != cdb)
        return true;

    return false;
}

bool projectPointOnLine(_Point &a, _Point &b, _Point &c, _Point &res)
{
  // get dot product of e1, e2
  _Point e1 (b.x() - a.x(), b.y() - a.y());
  _Point e2 (c.x() - a.x(), c.y() - a.y());

  float valDp = _Point::dot(e1, e2);
  // get squared length of e1
  float len2 = e1.len2();

  res.data[0] = (a.x() + (valDp * e1.x()) / len2);
  res.data[1] = (a.y() + (valDp * e1.y()) / len2);

  _BoundingBox bb(a, b);

  return bb.contains(res);
}

bool t2d2::util::intersects(float *a, float *b, float *c, float *d)
{
    _Point ap(a);
    _Point bp(b);
    _Point cp(c);
    _Point dp(d);

    return isIntersection(ap, bp, cp, dp);
}

bool t2d2::util::pointToSegmentProjection(float *a, float *b, float *c, float *res)
{
    _Point ap(a);
    _Point bp(b);
    _Point cp(c);
    _Point rp(res);

    return projectPointOnLine(ap, bp, cp, rp);
}

bool t2d2::util::pointOnSegment(float *a, float *b, float *c)
{
    _Point _a(a);
    _Point _b(b);
    _Point _c(c);

    if (_a ==_c || _b == _c) {
        return true;
    }

    _Point res(0,0);

    if (!projectPointOnLine(_a, _b, _c, res))
        return false;

    res.data[0] = _c.x() - res.x();
    res.data[1] = _c.y() - res.y();

    if (almostEqual2sComplement(res.len2(), 0.0f, 1))
        return true;
    return false;
}


bool t2d2::util::pointOnContour(float *poly, int length, int stride, float *point)
{
    float *pC = point;
//    float pD[2];
//    pD[1] = pC[1];

    for(int i = 0; i < length; i++)
    {
        int j = _index(i+1, length);

        float *pA = poly + i * stride;
        float *pB = poly + j * stride;

        if (pointOnSegment(pA, pB, pC)) {
            return true;
        }
    }
    return false;
}


bool t2d2::util::contourContains(float *poly, int length, int stride, float *point)
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

        if (pointOnSegment(pA, pB, pC))
            return true;

        pD[0] = std::max(pA[0], pB[0]);

        if (pD[0] < pC[0]) {
            continue;
        }

        pD[0] += 10;

        if (intersects(pA, pB, pC, pD)) {
            c++;
        } else {
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



                oCAB = orientation(_Point(pC), _Point(_pA), _Point(_pB));
                in++;
            }

            while (oCAPrA == 0) {

                ib = _index (ib, length);
                int jb = _index (ib + 1, length);

                float *_pA = poly + ib * stride;
                float *_pB = poly + jb * stride;
                oCAPrA = orientation(_Point(pC), _Point(_pB), _Point(_pA));
                ib--;
            }

            if (oCAB == oCAPrA) {
                c+=2;
            } else {
                c++;
            }
        }
    }
    return (c%2) == 1;
}

int t2d2::util::contourEdgeSelfIntersection(float *poly, int length, int stride, int index)
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

int t2d2::util::segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC)
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

int t2d2::util::findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out)
{
    float d2;
    int index = -1;
    _Point outP(0,0);
    _Point delta(0,0);

    for(int i = 0; i < length; i++) {
        float *pA = contour + i * stride;
        float *pB = contour + _index (i + 1, length) * stride;

        _Point a(pA);
        _Point b(pB);
        _Point c(point);
        _Point res(0,0);

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

    if (out != 0 && index >= 0) {
        out[0] = outP.x();
        out[1] = outP.y();
    }

    return index;
}

bool _pointOnSegment(_Point &pA, _Point &pB, _Point &pC)
{
    _BoundingBox bb(pA, pB);

    if (!bb.contains(pC))
        return false;
    int o = orientation(pC, pA, pB);

    if (o == 0)
        return true;

    return false;
}

bool util::hasContourEdgeSelfIntersection(t2d2::Contour *contour)
{
    unsigned int l = contour->length();

    for(unsigned int i = 0; i < l; i++) {

        _Point pa((*contour)[i]);
        _Point pb((*contour)[i + 1]);

        int is = _index (i + 2, l);
        int ie = _index (i -1, l);

        while (is != ie) {

            _Point pc((*contour)[is]);
            _Point pd((*contour)[is + 1]);

            if (isIntersection (pa, pb, pc, pd)) {
                Log()<<__FUNCTION__<<"Contour self intersection detected. Size: "<<l;

                Log()<<__FUNCTION__<<"pa"<<pa.x()<<pa.y()<<"  pb"<<pb.x()<<pb.y()<<"  indices:"<<i<<i+1;
                Log()<<__FUNCTION__<<"pc"<<pc.x()<<pc.y()<<"  pd"<<pd.x()<<pd.y()<<"  indices:"<<is<<is+1;

                return true;
            }

            is = _index(is + 1, l);
        }
    }
    return false;
}

bool util::contourContainsSegment(t2d2::Contour *contour, t2d2::Point *pointA, t2d2::Point *pointB, bool segmentIntersectionRule)
{
    _Point pC(pointA);
    _Point pCR(pointA);
    _Point pD(pointB);


    int c = 0;
    int length = contour->length();

    for(int i = 0; i < length; i++)
    {
        // set next segment as pA & pB
        _Point pA((*contour)[i]);
        _Point pB((*contour)[i+1]);

        // check point on Segment

        if (isIntersection(pA, pB, pC, pD))
            return segmentIntersectionRule;

        // set pD.x to max X of segment
        pCR.setX(std::max(pA.x(), pB.x()));


        // if pD.x is less then pC.x - segment is at left of point, ignore it
        if (pCR.x() < pC.x()) {
            continue;
        }

        // make pD as ray outside the segment
        pCR.setX(pCR.x() + 10);


        if (isIntersection(pA, pB, pC, pCR)) {
            c++;
        } else {
            continue;
        }


        // what if the pA is laying on the ray
        if (_pointOnSegment(pC, pCR, pA)) {

            int oCAB, oCAPrA;
            oCAB = oCAPrA = 0;

            int in = _index(i, length);
            int ib = _index (i - 1, length);

            while (oCAB == 0) {
                in = _index (in, length);

                _Point _pA((*contour)[in]);
                _Point _pB((*contour)[in + 1]);

                oCAB = orientation(pC, _pA, _pB);
                in = _index (in + 1, length);
            }

            while (oCAPrA == 0) {
                ib = _index (ib, length);

                _Point _pA((*contour)[ib]);
                _Point _pB((*contour)[ib + 1]);

                oCAPrA = orientation(pC, _pB, _pA);
                ib = _index (ib - 1, length);
            }

            if (oCAB == oCAPrA) {
                c+=2;
            } else {
                c++;
            }
        }
    }
    return (c%2) == 1;
}

bool util::isHoleContourValid(Contour *holeContour, Contour *contour)
{
    //bbox check
    if (!contour->bbox()->contains(holeContour->bbox()))
        return false;

    unsigned int l = holeContour->length();

    for(unsigned int i = 0; i < l; i++) {

        t2d2::Point *iPA = dynamic_cast<t2d2::Point*>((*holeContour)[i]);
        t2d2::Point *iPB = dynamic_cast<t2d2::Point*>((*holeContour)[i+1]);

        //hole point is placed inside the main contour, exclude contour

        if (!contourContainsSegment (contour, iPA, iPB, false))
            return false;

        _Point pa((*holeContour)[i]);
        _Point pb((*holeContour)[i + 1]);

        //self intersection check

        int is = _index (i + 2, l);
        int ie = _index (i -1, l);

        while (is != ie) {
            _Point pc((*holeContour)[is]);
            _Point pd((*holeContour)[is + 1]);

            if (isIntersection (pa, pb, pc, pd))
                return false;
            is = _index(is + 1, l);
        }

    }
    return true;
}

void t2d2::util::getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax)
{
    float xmin = 1e38f;
    float xmax = -1e37f;
    float ymin = 1e38f;
    float ymax = -1e37f;

    for(int i = 0; i < length; i++) {
        _Point p(contour);
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

void util::getAveargePoint(float *contour, int length, int stride, float *outX, float *outY)
{
    if (length == 0)
        return;

    float sx = 0;
    float sy = 0;

    for(int i = 0; i < length; i++) {
        sx += contour[0];
        sy += contour[1];
        contour += stride;
    }

    *outX = sx / length;
    *outY = sy / length;
}


float util::triOrient(Point *pA, Point *pB, Point *pC)
{
    return orient(pA->x, pA->y, pB->x, pB->y, pC->x, pC->y);
}

float util::triArea(Point *pA, Point *pB, Point *pC)
{
    float xa = pB->x - pA->x;
    float ya = pB->y - pA->y;

    float xb = pC->x - pB->x;
    float yb = pC->y - pB->y;

    float xc = pA->x - pC->x;
    float yc = pA->y - pC->y;


    float a = sqrtf (xa * xa + ya * ya);
    float b = sqrtf (xb * xb + yb * yb);
    float c = sqrtf (xc * xc + yc * yc);

    float p = (a+b+c)/2;
    float da = p * (p - a) * (p -  b) * (p - c);

    return sqrtf(da);
}

void util::triMidPoint(Point *pA, Point *pB, Point *pC, float *outX, float *outY)
{
    *outX = (pA->x + pB->x + pC->x) / 3;
    *outY = (pA->y + pB->y + pC->y) / 3;
}


void util::avrPoint(float *in, unsigned int sIndex, unsigned int wsize, float &avrX, float &avrY)
{
    avrX = avrY = 0;

    for(unsigned int i = 0; i < wsize; i++) {
        unsigned int index = (i + sIndex) * 2;
//        Log()<<__FUNCTION__<<"      proc idx: "<<index<<"  x: "<<in [index]<<" y: "<<in [index + 1];
        avrX += in [index];
        avrY += in [index + 1];
    }

    avrX /= wsize;
    avrY /= wsize;

//    Log()<<__FUNCTION__<<" x: "<<avrX<<" y: "<<avrY;
}


void util::avrPointOnClosedContour(float *in, unsigned int len, unsigned int sIndex, unsigned int wsize, float &avrX, float &avrY)
{
    avrX = avrY = 0;

    for(unsigned int i = 0; i < wsize; i++) {

        unsigned int index = _idx((i + sIndex), len) * 2;
//        Log()<<__FUNCTION__<<"      proc idx: "<<index<<"  x: "<<in [index]<<" y: "<<in [index + 1];
        avrX +=  in [index];
        avrY +=  in [index + 1];
    }

    avrX /= wsize;
    avrY /= wsize;

//    Log()<<__FUNCTION__<<" x: "<<avrX<<" y: "<<avrY;
}

bool util::averagePolygon(float *in, unsigned int len, float **out, unsigned int &lenOut, unsigned int wsize, unsigned int step)
{
//    Log()<<__FUNCTION__<<" in.len: "<<len<<" wsize: "<<wsize<<" step: "<<step;

    if (step == 0)
        return false;
    if (wsize >= len)
        return false;

    std::vector<float> outVec;

    unsigned int ei = len - wsize;

    float prevX, prevY;

    avrPoint(in, 0, wsize, prevX, prevY);

    outVec.push_back(prevX);
    outVec.push_back(prevY);

    for(unsigned int i = 1; i < ei; i+= step) {
        float x, y;

//        Log()<<__FUNCTION__<<" i: "<<i;

        avrPoint(in, i, wsize, x, y);

        prevX = x;
        prevY = y;

        outVec.push_back(x);
        outVec.push_back(y);
    }

    for(unsigned int i = ei; i < len; i+= step) {
        float x, y;

//        Log()<<__FUNCTION__<<" i: "<<i;

        avrPointOnClosedContour(in, len, i, wsize, x, y);

        prevX = x;
        prevY = y;

        outVec.push_back(x);
        outVec.push_back(y);
    }

    int os = outVec.size();

//    Log()<<__FUNCTION__<<" output len: "<<os/2;

    if (os < 6)
        return false;

    float *oa = new float[os];

    float * p = oa;
    for(int i = 0; i < os; i++ )
        *p++ = outVec[i];
    lenOut = os / 2;
    *out = oa;
    return true;
}

void util::cl::normal(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float &nx, float &ny)
{
    if(p1.X == p0.X && p1.Y == p0.Y) {
        nx = ny = 0.0f;
    }
    float dx = static_cast<float>(p1.X - p0.X);
    float dy = static_cast<float>(p1.Y - p0.Y);
    float f = 1 * 1.0f/ sqrtf (dx*dx + dy*dy);
    dx *= f;
    dy *= f;

    nx = dy;
    ny = -dx;
}

void util::cl::normalOffsetP0(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float delta, ClipperLib::IntPoint &out)
{
    float nx, ny;
    util::cl::normal(p0, p1, nx, ny);
    out.X = p0.X + static_cast<ClipperLib::cInt>(roundf(nx * delta));
    out.Y = p0.Y + static_cast<ClipperLib::cInt>(roundf(ny * delta));

}

void util::cl::normalOffsetP1(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float delta, ClipperLib::IntPoint &out)
{
    float nx, ny;
    util::cl::normal(p0, p1, nx, ny);
    out.X = p1.X + static_cast<ClipperLib::cInt>(roundf(nx * delta));
    out.Y = p1.Y + static_cast<ClipperLib::cInt>(roundf(ny * delta));
}

void util::cl::miter(ClipperLib::IntPoint &pp, ClipperLib::IntPoint &p, ClipperLib::IntPoint &np, float delta, ClipperLib::IntPoint &outMiter)
{
    float nx0;
    float ny0;

    float nx1;
    float ny1;

    util::cl::normal(pp, p, nx0, ny0);
    util::cl::normal(p, np, nx1, ny1);

    float r = 1.0f + (nx0 * nx1 + ny0 * ny1);
    float q =  delta / r;

    outMiter.X = static_cast<ClipperLib::cInt>(roundf((nx0 + nx1) * q));
    outMiter.Y = static_cast<ClipperLib::cInt>(roundf((ny0 + ny1) * q));
}

void util::cl::offsetPoint(ClipperLib::Path &path, size_t index, bool open, float delta, ClipperLib::IntPoint &out)
{
    size_t ps = path.size();

    if (ps < 2)
        return;

    if (index == 0) {
        if (open) {
            util::cl::normalOffsetP0(path[0], path[1], delta, out);
        } else {
            util::cl::miter(path[ps-1], path[0], path[1], delta, out);
        }
    } else if (index == ps-1) {
        if (open) {
            util::cl::normalOffsetP1(path[ps-2], path[ps-1], delta, out);
        } else {
            util::cl::miter(path[ps-2], path[ps-1], path[0], delta, out);
        }
    } else {
        util::cl::miter(path[index-1], path[index], path[index+1], delta, out);
    }
}

void util::cl::findNearest(ClipperLib::Path &path, ClipperLib::Path &targets, float distTolerance, std::vector<int> &out)
{
    distTolerance *= distTolerance;

    size_t ts = targets.size();

    out.clear();
    out.resize(ts);
    std::vector<float> minDst;
    minDst.reserve(ts);

    for(size_t i = 0; i < ts; i++) {
        out[i] = -1;
        minDst[i] = distTolerance;
    }

    size_t ps = path.size();

    for(size_t i = 0; i < ps; i++) {
        ClipperLib::IntPoint &p = path[i];
        for(size_t t = 0; t < ts; t++) {
            ClipperLib::IntPoint &cp = targets[t];
            float md = util::cl::dist2(cp, p);
            if( md <= minDst[t]) {
                minDst[t] = md;
                out[t] = i;
            }
        }
    }
}

float util::dist2(Point *pA, Point *pB)
{
    float dx = pB->x - pA->x;
    float dy = pB->y - pA->y;
    return dx * dx + dy * dy;
}

float util::dist(Point *pA, Point *pB)
{
    float dx = pB->x - pA->x;
    float dy = pB->y - pA->y;
    return  sqrtf(dx * dx + dy * dy);
}

int util::cl::findNearest(ClipperLib::Path &path, ClipperLib::IntPoint &target, float distTolerance)
{
    int res = -1;

    distTolerance *= distTolerance;
    float minDst = distTolerance;

    size_t ps = path.size();

    for(size_t i = 0; i < ps; i++) {
        ClipperLib::IntPoint &p = path[i];
        float md = util::cl::dist2(target, p);
        if( md <= minDst) {
            minDst = md;
            res = i;
        }
    }

    return res;
}

#ifndef ANDROID

void util::cl::savePath(ClipperLib::Path &p, const std::string &fileName)
{
    std::ofstream fs;
    fs.open(fileName.c_str(), std::ios::out | std::ios::trunc);

    if (!fs.is_open()) {
        Log(ltError)<<"unable to open:"<<fileName;
        return;
    }

    fs<<p.size()<<std::endl;

    for(size_t i = 0; i < p.size(); i++) {
        ClipperLib::IntPoint &pnt = p[i];
        fs<<pnt.X<<" "<<pnt.Y<<std::endl;
    }

    fs.close();
}

#endif

