#include "polygongroup.h"

using namespace t2d2;

PolygonGroup::PolygonGroup() :
    m_polygon(nullptr)
{
    m_borders = new Borders();
}

PolygonGroup::~PolygonGroup()
{
    delete m_borders;
    deletePolygons();
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

MCash *PolygonGroup::createMCash(t2d2::MCashContentOptions mcocOpt, t2d2::MCashStageOptions mcosOpt, int stride, int subMeshNum)
{
    MCash *mcash = new MCash(this);

    mcash->allocate(mcocOpt, mcosOpt, stride, subMeshNum);

    return mcash;
}

void PolygonGroup::deleteMCash(MCash *mcash)
{
    if (mcash)
        delete mcash;
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

    t2d2::Borders::saveToFile(pg->m_borders, fs);
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

    t2d2::Borders::loadFromFile(pg->m_borders, fs);

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
