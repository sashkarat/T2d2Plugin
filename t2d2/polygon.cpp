#include "polygon.h"


using namespace t2d2;

int Polygon::cashTriOffset() const
{
    return m_cashTriOffset;
}

void Polygon::setCashTriOffset(int cashTriOffset)
{
    m_cashTriOffset = cashTriOffset;
}

Polygon::Polygon(PolygonGroup *pg) : m_first(this), m_prev(nullptr), m_next(nullptr)
{
    m_bbox = new BBox();
    m_triangles = nullptr;
    m_triangleNum = 0;

    m_polyGroup = pg;
    m_contour = new Contour(this, true);

    m_zValue = 0.0f;
    m_subMeshIndex = 0;
    m_cashTriOffset = -1;

    m_genMesh = true;
    m_genCollider = false;
    m_clippingSubj = true;
    m_clippingClip = false;
}

Polygon::~Polygon()
{
    deleteTriangles();

    delete m_bbox;
    delete m_contour;
    for(unsigned int i = 0; i < m_holes.size(); i++)
        delete m_holes[i];
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

Polygon *Polygon::findLast()
{
    Polygon *p = this;
    while(p->m_next != nullptr)
        p = p->m_next;
    return  p;
}

Contour *Polygon::contour()
{
    return m_contour;
}

int Polygon::holesCount()
{
    return static_cast<int>(m_holes.size());
}

Contour *Polygon::hole(unsigned int index)
{
    if (index >= m_holes.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index is out of range";
        return nullptr;
    }
    return m_holes[index];
}

Contour *Polygon::addHole()
{
    m_holes.push_back( new Contour(this));
    return m_holes [m_holes.size() - 1];
}

void Polygon::deleteHole(unsigned int index)
{
    if (index >= m_holes.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index is out of range";
        return;
    }
    delete m_holes[index];
    m_holes.erase(m_holes.begin()+index);
}

unsigned int Polygon::triNumber()
{
    return m_triangleNum;
}

Polygon::Triangle *Polygon::tri(int index)
{
    return &m_triangles[index];
}

void Polygon::validate()
{
    m_contour->validate();
    for(int i = 0; i < m_holes.size(); i++)
        m_holes[i]->validate();
}

void Polygon::triangulate()
{
    deleteTriangles();

    p2t::CDT *p2tCdt = new p2t::CDT(m_contour->m_data);

    for(int i = 0; i < m_holes.size(); i++)
        p2tCdt->AddHole(m_holes[i]->m_data);

    p2tCdt->Triangulate();

    std::vector<p2t::Triangle *> &tri = p2tCdt->GetTriangles();

    m_triangleNum = static_cast<unsigned int>(tri.size());

    m_triangles = new Triangle[m_triangleNum];



    Triangle *pt = m_triangles;

    for(unsigned int i = 0; i < m_triangleNum; i++) {
        p2t::Triangle *st = tri[i];
        pt->points[0] = dynamic_cast<t2d2::Point*>(st->GetPoint(0));
        pt->points[1] = dynamic_cast<t2d2::Point*>(st->GetPoint(1));
        pt->points[2] = dynamic_cast<t2d2::Point*>(st->GetPoint(2));
        pt++;
    }

    delete p2tCdt;
}

void Polygon::deleteTriangles()
{
    if (m_triangles)
        delete []m_triangles;
    m_triangles = nullptr;
    m_triangleNum = 0;
}

void Polygon::updateBBox()
{
    m_bbox->reset();
    for(unsigned int i = 0; i < m_contour->m_data.size(); i++)
        m_bbox->update(dynamic_cast<t2d2::Point*>(m_contour->m_data[i]));
}

BBox &Polygon::bbox()
{
    return *m_bbox;
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

    if (pp!= nullptr)
        pp->m_next = pn;

    if (pn!= nullptr) {
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

void Polygon::updateFirst()
{
    Polygon *p = m_next;
    while(p!= nullptr) {
        p->m_first = p->m_prev->m_first;
        p = p->m_next;
    }
}

