#include "polygon.h"


using namespace t2d2;

//======================================================================================

Polygon *PolygonGroup::reset()
{
    clean();
    m_polygon = new Polygon();
    return m_polygon;
}

void PolygonGroup::clean()
{
    while (m_polygon != nullptr) {
        Polygon *n = m_polygon->next();
        delete m_polygon;
        m_polygon = n;
    }
    m_polygon = nullptr;
}

//=======================================================================================

Polygon::Polygon() : m_first(this), m_prev(nullptr), m_next(nullptr)
{

}

Polygon::~Polygon()
{
    cleanPoints(m_contour);
    for(int i = 0; i < m_holes.size(); i++) {
        Points &points = m_holes[i];
        cleanPoints (points);
    }
}

Polygon *Polygon::prev()
{
    return m_prev;
}

Polygon *Polygon::next()
{
    return m_next;
}

Polygon *Polygon::first()
{
    return m_first;
}


void Polygon::insertNext(Polygon *p)
{
        p->m_prev = this;
        p->m_next = m_next;
        if (m_next != nullptr)
            m_next->m_prev = p;
        m_next = p;
        p->m_first = m_first;
        p->updateFirst();
}

void Polygon::insertPrev(Polygon *p)
{
    p->m_next = this;
    p->m_prev = m_prev;
    if (m_prev == nullptr) {
        m_first = p->m_first;
        updateFirst();
    } else {
        m_prev->m_next = p;
    }
    m_prev = p;
}

void Polygon::exclude(Polygon *p)
{
    Polygon *pp = p->m_prev;
    Polygon *pn = p->m_next;

    if(pp!= nullptr)
        pp->m_next = pn;

    if(pn!= nullptr) {
        pn->m_prev = pp;
        if (pn->m_first == p) {
            pn->m_first = pn;
            pn->updateFirst();
        }
    }


    p->m_prev = nullptr;
    p->m_next = nullptr;
    p->m_first = p;
}

Points &Polygon::contour()
{
    return m_contour;
}

Points *Polygon::ptrContour()
{
    return &m_contour;
}

int Polygon::holesCount()
{
    return static_cast<int>(m_holes.size());
}

Points &Polygon::hole(int index)
{
    return m_holes[index];
}

Points *Polygon::ptrHole(int index)
{
    return & hole (index);
}

Points &Polygon::addHole()
{
    m_holes.push_back(Points());
    return m_holes [m_holes.size() - 1];
}

Points *Polygon::ptrAddHole()
{
    return & addHole ();
}

void Polygon::deleteHole(int index)
{
    Points &ps = m_holes[index];
    cleanPoints(ps);

    m_holes.erase(m_holes.begin()+index);
}

void Polygon::cleanPoints(Points &points)
{
    for(int j = 0; j< points.size(); j++)
        delete points[j];
    points.resize(0);
}

void Polygon::cleanPoints(Points &points, int index, int count)
{
    if ((index + count) >= points.size() )
        count = static_cast<int>(points.size()) - index;

    int e = index + count;
    for(int i = index; i < e; i++) {
        delete points[i];
        points[i] = nullptr;
    }
}

void Polygon::updateBBox()
{
    m_bbox.reset();
    for(int i = 0; i < m_contour.size(); i++)
        m_bbox.update(m_contour[i]);
}

const BBox &Polygon::bbox()
{
    return m_bbox;
}

void Polygon::updateFirst()
{
    Polygon *p = m_next;
    while(p!= nullptr) {
        p->m_first = p->m_prev->m_first;
        p = p->m_next;
    }
}

