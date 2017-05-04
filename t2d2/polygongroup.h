#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"

namespace t2d2 {

class Polygon;
class MCash;

class PolygonGroup
{

    Polygon *m_polygon;
    MCash   *m_mCash;

public :

    PolygonGroup() : m_polygon(nullptr), m_mCash(nullptr) {}
    ~PolygonGroup() { freeMCash(); deletePolygons(); }

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    void        deletePolygon(Polygon *p);

    void validate();

    void allocCash(int stride, int subMeshNum);

    MCash *mcash() const;

    void freeMCash();

    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);

protected:
    void deletePolygons();
};

}

#endif // POLYGONGROUP_H
