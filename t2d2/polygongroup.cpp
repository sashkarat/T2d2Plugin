#include "polygongroup.h"

using namespace t2d2;

PolygonGroup::PolygonGroup() :
    m_polygon(nullptr)
{
    m_borders = new Borders();
    m_colliderArea = 0;
    m_colliderComX = m_colliderComY = 0;
    m_colliderPathNum = 0;
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

MeshCash *PolygonGroup::createMeshCash(int subMeshNum)
{
    MeshCash *mcash = new MeshCash(this);
    mcash->build(subMeshNum);
    return mcash;
}

void PolygonGroup::deleteMeshCash(MeshCash *mcash)
{
    if (mcash)
        delete mcash;
}

void PolygonGroup::updateColliderGeometricValues()
{
    Polygon *poly = m_polygon;

    m_colliderComX = m_colliderComY = 0;
    m_colliderArea = 0;
    m_colliderPathNum = 0;

    while (poly != nullptr) {

        if (poly->genCollider()) {

            poly->updateCOM();

            float a = poly->getArea();
            float x = poly->getComX();
            float y = poly->getComY();


            m_colliderArea += a;

            m_colliderComX += x * a;
            m_colliderComY += y * a;

            m_colliderPathNum++;
        }

        poly = poly->next();
    }

    if (m_colliderPathNum == 0)
        return;

    if (m_colliderArea == 0) {
        m_colliderComX = m_colliderComY = 0;
        return;
    }

    m_colliderComX /= m_colliderArea;
    m_colliderComY /= m_colliderArea;
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

bool PolygonGroup::clipBy(PolygonGroup *clipperPg, float *trMtx)
{
    Polygon *clipperPoly = clipperPg->polygon();

    while (clipperPoly != nullptr) {

        if (clipperPoly->clippingClip()) {
            Polygon *cp = clipperPoly;

            if (trMtx != nullptr) {
                //TODO: make a transformed copy of the clipperPoly
            }

            clipBy(cp);
        }


        clipperPoly = clipperPoly->next();
    }
    return false;
}

void PolygonGroup::deletePolygons()
{
    while (m_polygon != nullptr) {
        Polygon *n = m_polygon->next();
        delete m_polygon;
        m_polygon = n;
    }
}

bool PolygonGroup::clipBy(Polygon *clipperPoly)
{
    std::vector<t2d2::Polygon*> prevPolyVec;
    std::vector<t2d2::Polygon*> newPolyVec;


    Polygon *poly = m_polygon;

    while (poly != nullptr) {
        if (!poly->clippingSubj()) {
            poly = poly->next();
            continue;
        }

        bool res = poly->clipBy(clipperPoly, newPolyVec);

        poly = poly->next();
    }

    return false;
}
