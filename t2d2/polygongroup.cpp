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

void PolygonGroup::validate()
{
    Polygon *poly = m_polygon;
    while(poly != nullptr) {
        poly->validate();
        poly = poly->m_next;
    }
}

void PolygonGroup::allocCash(int stride, int subMeshNum)
{
    if (m_mCash == nullptr)
        m_mCash = new MCash();
    m_mCash->allocate(stride, this, subMeshNum);
}

void PolygonGroup::freeMCash()
{
    if (m_mCash)
        delete m_mCash;
    m_mCash = nullptr;
}

void PolygonGroup::saveToFile(PolygonGroup *pg, std::ofstream &fs)
{
    Polygon *poly = pg->m_polygon;

    char nf = (poly)?1:0;

    fs.write(&nf, 1);

    while (poly != nullptr ) {
        Polygon::saveToFile(poly, fs);
        poly = poly->m_next;
        nf = (poly)?1:0;
        fs.write(&nf, 1);
    }
}

PolygonGroup *PolygonGroup::loadFromFile(std::ifstream &fs)
{

    PolygonGroup *pg = new PolygonGroup();
    char nf = 1;
    while (!fs.eof()) {
        fs.read(&nf, 1);
        if (nf) {
            Polygon *poly = pg->addPolygon();
            Polygon::loadFromFile(poly, fs);
        } else {
            break;
        }
    }
    return pg;
}

void PolygonGroup::deletePolygons()
{
    while (m_polygon != nullptr) {
        Polygon *n = m_polygon->next();
        delete m_polygon;
        m_polygon = n;
    }
}
