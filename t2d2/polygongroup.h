#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"
#include "clipperlib_f/clipper.hpp"


namespace t2d2 {

class SimpPolyData;
class SimpContourData;
class MCash;
class MeshCash;
class ColliderCash;
class Polygon;
class Borders;

class PolygonGroup
{
    Polygon *m_polygon;
    Borders *m_borders;

    float m_colliderArea;
    float m_colliderComX;
    float m_colliderComY;

    int m_colliderPathNum;

    SimpPolyData *m_clippingClip;

public :

    PolygonGroup();
    ~PolygonGroup();

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    Polygon*    addPolygon(t2d2::Polygon *poly);
    void addPolygons(std::vector<t2d2::Polygon *> &polyVec);
    void        deletePolygon(Polygon *p);
    Borders*    borders() {return m_borders;}

    MeshCash *createMeshCash(int subMeshNum, bool validate);
    void deleteMeshCash (MeshCash *mcash);

    ColliderCash *createColliderCash();
    void deleteColliderCash(ColliderCash * ccash);




    void updateColliderGeometricValues();

    inline float getColliderArea() const {return m_colliderArea;}
    inline void  getColliderCOM(float *x, float *y) const {*x = m_colliderComX; *y = m_colliderComY;}
    inline float getColliderComX() const {return m_colliderComX;}
    inline float getColliderComY() const {return m_colliderComY;}
    inline int   getColliderPathNum() const { return m_colliderPathNum;}

    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);


    bool clipBy(t2d2::PolygonGroup *clipperPg, float *trMtx);

    void addClippingClip(t2d2::PolygonGroup *pg, float *trMtx);

    bool clip();

    bool slice(Polygon *poly, int gridSize);


protected:

    void deletePolygons();


    bool clipBy(t2d2::SimpPolyData *spd);

    void clippingClearClipData();

    SimpPolyData *clippingFindLastClipData();

    bool clippingOverlapTest(SimpPolyData *spd);

    static void clippingAddClipToClipper(ClipperLib::Clipper &cl, SimpPolyData *spd);
    static void clippingAddSubjToClipper(ClipperLib::Clipper &cl, Polygon *poly);
};

}

#endif // POLYGONGROUP_H
