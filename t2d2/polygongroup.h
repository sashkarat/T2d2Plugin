#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"
#include "mcash.h"

namespace t2d2 {

enum MCashContentOptions;
enum MCashStageOptions;

class MCash;
class Polygon;

class PolygonGroup
{
    Polygon *m_polygon;

public :

    PolygonGroup() : m_polygon(nullptr) {}
    ~PolygonGroup() { deletePolygons(); }

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    void        deletePolygon(Polygon *p);


    MCash *createMCash (t2d2::MCashContentOptions mcocOpt, t2d2::MCashStageOptions mcosOpt, int stride , int subMeshNum);

    void deleteMCash (MCash *mcash);

    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);

protected:

    void deletePolygons();
};

}

#endif // POLYGONGROUP_H
