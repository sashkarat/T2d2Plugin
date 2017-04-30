#include "polygongroup.h"

using namespace t2d2;

MCash *PolygonGroup::mcash() const
{
    return m_mCash;
}

Polygon *PolygonGroup::addPolygon()
{
    if (m_polygon == nullptr) {
        m_polygon = new Polygon(this);
        return m_polygon;
    } else {
        Polygon *p = m_polygon->findLast();
        Polygon *np = new Polygon(this);
        p->insertNext(np);
        return np;
    }
}

void PolygonGroup::deletePolygon(Polygon *p)
{
    if (p == m_polygon)
        m_polygon = p->next();
    Polygon::exclude(p);
    delete p;
}

void PolygonGroup::allocCash(int stride, int subMeshNum)
{
    if (m_mCash == nullptr)
        m_mCash = new MCash();

    m_mCash->allocate(stride, this, subMeshNum);
}

void PolygonGroup::freeCash()
{
    if (m_mCash)
        delete m_mCash;
    m_mCash = nullptr;
}

void PolygonGroup::deletePolygons()
{
    while (m_polygon != nullptr) {
        Polygon *n = m_polygon->next();
        delete m_polygon;
        m_polygon = n;
    }
}
