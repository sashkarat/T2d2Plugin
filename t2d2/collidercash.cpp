#include "collidercash.h"
#include "clipperlib_f/clipper.hpp"

void t2d2::ColliderCash::clear()
{
    if (m_data)
        delete [] m_data;

    m_data = nullptr;

    m_colliderNum = 0;
}

t2d2::ColliderCash::ColliderCash(t2d2::PolygonGroup *pg) :
    m_pg(pg),
    m_colliderNum(0),
    m_data(nullptr)
{

}

void t2d2::ColliderCash::build()
{
    clear();

    m_area = m_pg->getColliderArea();
    m_comX = m_pg->getColliderComX();
    m_comY = m_pg->getColliderComY();

    t2d2::Polygon *poly = m_pg->polygon();
    while (poly != nullptr) {
        if (poly->genCollider())
            m_colliderNum++;
        poly = poly->next();
    }

    m_data = new ColliderData [m_colliderNum];

    int ci = 0;

    poly = m_pg->polygon();
    while (poly != nullptr) {
        if (poly->genCollider()) {
            m_data[ci].setContour(poly->outline());
            ci++;
        }
        poly = poly->next();
    }
}

void t2d2::ColliderCash::makeFixed()
{
    for (int i =0; i < m_colliderNum; i++)
        m_data[i].makeFixed();
}

void t2d2::ColliderCash::offset(float o)
{
    ClipperLib::ClipperOffset clipper ;

    for(int i = 0; i < m_colliderNum; i++) {
        ColliderData &cd = m_data[i];
        ClipperLib::Path p;
        if (cd.isFixed()) {
            Contour::makeClipperLibPath(p, cd.m_fixed->m_points, cd.m_fixed->m_len);
        } else {
            cd.m_contour->makeClipperLibPath(p);
        }
        clipper.AddPath(p, ClipperLib::jtMiter, ClipperLib::etClosedPolygon);
    }

    ClipperLib::PolyTree pt;

    clipper.Execute(pt, o * FLOAT2CLINT);

    rebuildFromClipperPolyTree(pt);
}

void t2d2::ColliderCash::average(unsigned int wsize, unsigned int step)
{
    for (int i = 0; i < m_colliderNum; i++) {
        ColliderData &cd = m_data[i];
        if (!cd.isFixed())
            cd.makeFixed();

        FixedColliderData *fxd = cd.m_fixed;

        float *newPoints;
        unsigned int newLen;

        bool res = t2d2::util::averagePolygon(fxd->m_points, fxd->m_len,
                                              &newPoints, newLen,
                                              wsize, step);
        if (!res)
            continue;
        cd.setAsFixed(newPoints, newLen);
    }
}

t2d2::ColliderCash::~ColliderCash()
{
    clear();
}

void t2d2::ColliderCash::rebuildFromClipperPolyTree(ClipperLib::PolyTree &pt)
{
    if (m_data)
        delete [] m_data;

    m_colliderNum = 0;

    ClipperLib::PolyNode *pnd = pt.GetFirst();

    while (pnd != nullptr) {
        if (!pnd->IsHole())
            m_colliderNum++;
         pnd = pnd->GetNext();
    }

    if (m_colliderNum == 0)
        return;

    m_data = new ColliderData[m_colliderNum];

    int ci = 0;

    pnd = pt.GetFirst();

    while (pnd != nullptr) {
        if (!pnd->IsHole()) {
            ColliderData &cd = m_data[ci++];
            cd.makeFixed(pnd->Contour);
        }
        pnd = pnd->GetNext();
    }
}

unsigned int t2d2::ColliderCash::ColliderData::len()
{
    if (m_fixed)
        return m_fixed->m_len;
    return m_contour->length();
}

void t2d2::ColliderCash::ColliderData::getPoints(float *out)
{
    if (m_fixed)
        return m_fixed->copy(out);
    m_contour->getValue2d(0, m_contour->length(), out);
}

bool t2d2::ColliderCash::ColliderData::isFixed()
{
    return m_fixed != nullptr;
}

void t2d2::ColliderCash::ColliderData::makeFixed()
{
    if (m_fixed)
        delete m_fixed;

    m_fixed = new FixedColliderData(m_contour->length());
    m_contour->getValue2d(0, m_contour->length(), m_fixed->m_points);
}

void t2d2::ColliderCash::ColliderData::makeFixed(ClipperLib::Path &path)
{
    if (m_fixed)
        delete m_fixed;

    size_t ps = path.size();

    m_fixed = new FixedColliderData(ps);

    for(size_t i = 0; i < ps; i++) {
        ClipperLib::IntPoint &p = path[i];
        m_fixed->set(i, p.X * CLINT2FLOAT, p.Y * CLINT2FLOAT);
    }
}

void t2d2::ColliderCash::ColliderData::setAsFixed(float *points, float len)
{
    if (m_fixed)
        delete m_fixed;
    m_fixed = new FixedColliderData(0);

    m_fixed->m_points = points;
    m_fixed->m_len = len;
}

t2d2::ColliderCash::FixedColliderData::FixedColliderData(unsigned int size)
{
    m_len = size;
    if (m_len != 0) {
        m_points = new float [m_len * 2];
    } else {
        m_points = nullptr;
    }
}

t2d2::ColliderCash::FixedColliderData::~FixedColliderData()
{
    if (m_len != 0)
        delete [] m_points;
}

void t2d2::ColliderCash::FixedColliderData::set(unsigned int index, float x, float y)
{
    index *=2;
    m_points[index] = x;
    m_points[index+1] = y;
}

void t2d2::ColliderCash::FixedColliderData::copy(float *out)
{
    memcpy(static_cast<void*>(out), static_cast<void*>(m_points), m_len*2 * sizeof(float));
}
