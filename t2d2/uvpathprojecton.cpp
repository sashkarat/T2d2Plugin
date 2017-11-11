#include "uvpathprojecton.h"



float t2d2::UvPathProjecton::dist(t2d2::UvPathProjecton::_UvpPoint &pA, t2d2::UvPathProjecton::_UvpPoint &pB)
{
    float dx = pB.x - pA.x;
    float dy = pB.y - pA.y;
    return sqrtf(dx * dx + dy * dy);
}

void t2d2::UvPathProjecton::build(ClipperLib::Path &p)
{
    alloc(p.size() + 1);


    m_p[0].x = p[0].X * CLINT2FLOAT;
    m_p[0].y = p[0].Y * CLINT2FLOAT;
    m_p[0].pos = 0;

    float accD = 0;
    float d =0;

//    Log()<<__FUNCTION__<<"i:"<<0<<"x"<<m_p[0].x<<"y"<<m_p[0].y<<"/ d"<<d;

    unsigned int psize = p.size();

    for(unsigned int i = 1; i < m_len; i++) {

        ClipperLib::IntPoint &point = p[i % psize];
        m_p[i].x = point.X * CLINT2FLOAT;
        m_p[i].y = point.Y * CLINT2FLOAT;

        d = dist(m_p[i-1], m_p[i]);
        m_p[i-1].d = d;
        accD += d;
//        Log()<<__FUNCTION__<<"i:"<<i<<"x"<<m_p[i].x<<"y"<<m_p[i].y<<"/ prev d"<<m_p[i-1].d;
    }

    float tLen = roundf(accD);

    float c = tLen / accD;
    m_p[0].d *= c;
//    Log()<<__FUNCTION__<<"i"<<0<<"d:"<<m_p[0].d<<"pos"<<m_p[0].pos;
    for(unsigned int i = 1; i < m_len; i++) {
        m_p[i].d *= c;
        m_p[i].pos = m_p[i-1].pos + m_p[i-1].d;
//        Log()<<__FUNCTION__<<"i"<<i<<"d:"<<m_p[i].d<<"pos"<<m_p[i].pos;
    }

    m_clPath.clear();
    m_clPath.insert(m_clPath.begin(), p.begin(), p.end());
}

int t2d2::UvPathProjecton::findNearest(float x, float y)
{
    _UvpPoint p;
    p.x = x;
    p.y = y;

    float minD = 1e32f;
    int idx = -1;

    for(unsigned int i = 0; i < m_len; i++) {
        float d = dist(m_p[i], p);
        if ( d < minD) {
            minD = d;
            idx = i;
        }
    }

    return idx;
}

bool t2d2::UvPathProjecton::isInside(float x, float y)
{
    ClipperLib::IntPoint p;
    p.X = x * FLOAT2CLINT;
    p.Y = y * FLOAT2CLINT;
    return isInside(p);
}

bool t2d2::UvPathProjecton::isInside(ClipperLib::IntPoint &p)
{
    return isInside(m_clPath, p);
}

void t2d2::UvPathProjecton::computeZControlPoints(float bwidth, float holeFactor,
                                                 ClipperLib::IntPoint &oS, ClipperLib::IntPoint &oE,
                                                 ClipperLib::IntPoint &wS, ClipperLib::IntPoint &wE)
{
    if (m_clPath.size() < 3)
        return;

    ClipperLib::IntPoint &p0 = m_clPath[0];
    ClipperLib::IntPoint &p1 = m_clPath[1];
    ClipperLib::IntPoint &pL = m_clPath[m_clPath.size()-1];

//    Log()<<__FUNCTION__<<"p1"<<p1.X<<p1.Y;
//    Log()<<__FUNCTION__<<"p0"<<p0.X<<p0.Y;
//    Log()<<__FUNCTION__<<"pL"<<pL.X<<pL.Y;


    ClipperLib::IntPoint miter;
    float delta = (bwidth / 2 * FLOAT2CLINT);

    t2d2::util::cl::miter(pL, p0, p1, delta, miter);


//    Log()<<__FUNCTION__<<"miter"<<miter.X<<miter.Y;

    ClipperLib::IntPoint pW;
    ClipperLib::IntPoint pO;

    pW.X = p0.X + miter.X * holeFactor;
    pW.Y = p0.Y + miter.Y * holeFactor;

    pO.X = p0.X - miter.X * holeFactor;
    pO.Y = p0.Y - miter.Y * holeFactor;

//    Log()<<__FUNCTION__<<"pO"<<pO.X<<pO.Y;
//    Log()<<__FUNCTION__<<"pW"<<pW.X<<pW.Y;

    t2d2::util::cl::normalOffsetP0(pW, pO, 10, wS);
    t2d2::util::cl::normalOffsetP1(pW, pO, 10, oS);
    t2d2::util::cl::normalOffsetP0(pO, pW, 10, oE);
    t2d2::util::cl::normalOffsetP1(pO, pW, 10, wE);


//    Log()<<__FUNCTION__<<"oS. X"<<oS.X<<"Y"<<oS.Y;
//    Log()<<__FUNCTION__<<"oE. X"<<oE.X<<"Y"<<oE.Y;
//    Log()<<__FUNCTION__<<"wS. X"<<wS.X<<"Y"<<wS.Y;
//    Log()<<__FUNCTION__<<"wE. X"<<wE.X<<"Y"<<wE.Y;
}

bool t2d2::UvPathProjecton::createZeroPointMiterClip(ClipperLib::Path &res, float bwidth)
{
    if (m_clPath.size() < 3)
        return false;

    ClipperLib::IntPoint &p0 = m_clPath[0];
    ClipperLib::IntPoint &p1 = m_clPath[1];
    ClipperLib::IntPoint &pL = m_clPath[m_clPath.size()-1];


    ClipperLib::IntPoint miter;
    float delta = (bwidth / 2 * FLOAT2CLINT);

    t2d2::util::cl::miter(pL, p0, p1, delta, miter);

    float mnx = miter.X;
    float mny = miter.Y;

    t2d2::util::fastnorm(mnx, mny);

    miter.X += mnx * 100;
    miter.Y += mny * 100;


    ClipperLib::Path bpath;

    bpath.push_back(ClipperLib::IntPoint(p0.X - miter.X, p0.Y - miter.Y));
    bpath.push_back(ClipperLib::IntPoint(p0.X + miter.X, p0.Y + miter.Y));

    ClipperLib::ClipperOffset co;

    co.AddPath(bpath, ClipperLib::jtMiter, ClipperLib::etOpenButt);

    ClipperLib::Paths sol;

    co.Execute(sol, 4.0f);

    if (sol.size() < 1)
        return false;

    res.clear();

    res.insert(res.begin(), sol[0].begin(), sol[0].end());

    return true;
}

bool t2d2::UvPathProjecton::isInside(ClipperLib::Path &path, ClipperLib::IntPoint &p)
{
    int res = ClipperLib::PointInPolygon(p, path);
    if (res == 0)
        return false;
    return true;
}
