#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"


namespace t2d2 {

enum MCashContentOptions;
enum MCashStageOptions;

class MCash;
class MeshCash;
class Polygon;
class Borders;

class PolygonGroup
{
    Polygon *m_polygon;
    Borders *m_borders;

    float m_colliderArea;
    float m_colliderComX;
    float m_colliderComY;


public :

    PolygonGroup();
    ~PolygonGroup();

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    void        deletePolygon(Polygon *p);
    Borders*    borders() {return m_borders;}

    MeshCash *createMeshCash(int subMeshNum);
    void deleteMeshCash (MeshCash *mcash);

    void updateColliderGeometricValues();

    inline float getColliderArea() const {return m_colliderArea;}
    inline void getColliderCOM(float *x, float *y) const {*x = m_colliderComX; *y = m_colliderComY;}
    inline float getColliderComX() const {return m_colliderComX;}
    inline float getColliderComY() const {return m_colliderComY;}


    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);

protected:

    void deletePolygons();
};

}

#endif // POLYGONGROUP_H
