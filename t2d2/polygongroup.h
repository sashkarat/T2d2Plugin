#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"


namespace t2d2 {

enum MCashContentOptions;
enum MCashStageOptions;

class MCash;
class Polygon;
class Borders;

class PolygonGroup
{
    Polygon *m_polygon;
    Borders *m_borders;

public :

    PolygonGroup();
    ~PolygonGroup();

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    void        deletePolygon(Polygon *p);
    Borders*    borders() {return m_borders;}

    MCash *createMCash (t2d2::MCashContentOptions mcocOpt, t2d2::MCashStageOptions mcosOpt, int stride , int subMeshNum);

    void deleteMCash (MCash *mcash);

    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);

protected:

    void deletePolygons();
};

}

#endif // POLYGONGROUP_H
