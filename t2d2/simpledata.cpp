#include "simpledata.h"

t2d2::SimpContourData::SimpContourData()
    : m_len(0), m_points(0) {}

t2d2::SimpContourData::~SimpContourData()
{
    if (m_points != 0 )
        delete [] m_points;
}

void t2d2::SimpContourData::set(t2d2::Contour *cntr)
{
    m_len = cntr->length();
    m_points = new float[m_len * 2];
    cntr->getValue2d(0, m_len, m_points);
}

/*
        pr2.x = p.x * m [0] + p.y * m [4] + p.z * m [8] + m [12];
        pr2.y = p.x * m [1] + p.y * m [5] + p.z * m [9] + m [13];
        pr2.z = p.x * m [2] + p.y * m [6] + p.z * m [10] + m [14];
*/

void t2d2::SimpContourData::applyTrMatrix(float *m)
{
    float *p = m_points;
    for(int i = 0; i < m_len; i++) {
        float x = *p;
        float y = *(p+1);

        float _x = x * m[0] + y * m[4] + m[12];
        float _y = x * m[1] + y * m[5] + m[13];

        *p++ = _x;
        *p++ = _y;
    }
}

void t2d2::SimpContourData::createClipperLibPath()
{
    m_clPath.clear();
    Contour::makeClipperLibPath(m_clPath, *this);
}

void t2d2::SimpContourData::alloc4Points()
{
    m_points = new float [8];
    m_len = 4;
}

void t2d2::SimpContourData::set(int index, float x, float y)
{
    index *= 2;
    m_points[index] = x;
    m_points[index + 1 ] = y;
}

float t2d2::SimpContourData::getX(int index)
{
    return m_points[index * 2];
}

float t2d2::SimpContourData::getY(int index)
{
    return m_points[index * 2 + 1];
}

t2d2::SimpPolyData::SimpPolyData(int hc):
    m_next(nullptr),
    m_hc(hc), m_bbox(0)
{
    if (m_hc==0)
        m_holes = nullptr;
    else
        m_holes = new SimpContourData [m_hc];
}

t2d2::SimpPolyData::~SimpPolyData()
{
    if (m_hc > 0)
        delete [] m_holes;
    if (m_bbox)
        delete m_bbox;
}

void t2d2::SimpPolyData::applyTrMatrix(float *m)
{
    m_outline.applyTrMatrix(m);
    for(int i = 0; i < m_hc; i++)
        m_holes[i].applyTrMatrix(m);
}

void t2d2::SimpPolyData::updateBBox()
{
    if (m_bbox)
        delete m_bbox;
    m_bbox = new BBox();
    float *p = m_outline.m_points;
    for(int i = 0; i < m_outline.m_len; i++){
        m_bbox->addPointSimp(*p, *(p+1));
        p+=2;
    }
}

void t2d2::SimpPolyData::createClipperLibPaths()
{
    ClipperLib::Path p;
    m_outline.createClipperLibPath();
    for(int i = 0; i < m_hc; i++) {
        m_holes[i].createClipperLibPath();
    }
}
