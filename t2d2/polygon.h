#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <fstream>
#include "t2d2.h"
#include "../poly2tri_f/common/shapes.h"

namespace t2d2 {

class Point;
class BBox;
class GridIndexator;
class Contour;
class PolygonGroup;
class UvProjection;


typedef t2d2::Point*                    PointPtr;
typedef t2d2::Contour*                  ContourPtr;
typedef std::vector<t2d2::ContourPtr>   ContourPtrVec;

class Polygon
{
    friend class PolygonGroup;

public:
    struct Triangle {
        PointPtr points [3];
    };

protected:
    PolygonGroup*           m_polyGroup;

    Contour*                m_outline;
    ContourPtrVec           m_holes;

    UvProjection*           m_uvProjection;

    Triangle*               m_triangles;
    unsigned int            m_triangleNum;


    float                   m_zValue;

    float                   m_area;
    float                   m_comX;
    float                   m_comY;

    float                   m_pivotX;
    float                   m_pivotY;

    unsigned int            m_subMeshIndex;
    int                     m_cashTriOffset;

    bool                m_genMesh;
    bool                m_genBorders;
    bool                m_genCollider;
    bool                m_clippingSubj;
    bool                m_clippingClip;

    Polygon *m_first;
    Polygon *m_prev;
    Polygon *m_next;

public:

    Polygon(PolygonGroup *pg);
    ~Polygon();

    Polygon *prev();
    Polygon *next();
    Polygon *first();
    Polygon *findLast();

    Contour *outline();

    int     holesCount();
    Contour* hole(unsigned int index);
    Contour* addHole();
    void    deleteHole(unsigned int index);

    unsigned int triNumber();
    Polygon::Triangle*  tri(int index);

    bool validate       (bool withHoles);
    void triangulate    (bool updateAreaAndCOM, bool allocTriangles, bool withHoles);
    void deleteTriangles();

    float            zValue()                   {return m_zValue;}
    unsigned int     subMeshIndex()             {return m_subMeshIndex;}

    void    setZValue(float v)                  {m_zValue = v;}
    void    setSubMeshIndex(unsigned int smi)   {m_subMeshIndex = smi;}

    void updateBBox();
    void updateIndexator(int gridSize);
    void updateBorderGeometry();

    BBox *bbox();

    t2d2::Point* findPoint(float x, float y);

    UvProjection*   getUvProjection() {return m_uvProjection;}

    bool isValid() const;

    inline bool genMesh()      const {return m_genMesh;}
    inline bool genBorders()   const {return m_genBorders;}
    inline bool genCollider()  const {return m_genCollider;}
    inline bool clippingSubj() const {return m_clippingSubj;}
    inline bool clippingClip() const {return m_clippingClip;}

    inline void setGenMesh         (bool s) {m_genMesh = s;}
    inline void setGenBorders      (bool s) {m_genBorders = s;}
    inline void setGenCollider     (bool s) {m_genCollider = s;}
    inline void setClippingSubj    (bool s) {m_clippingSubj = s;}
    inline void setClippingClip    (bool s) {m_clippingClip = s;}

    inline int cashTriOffset() const                {return m_cashTriOffset;}
    inline void setCashTriOffset(int cashTriOffset) {m_cashTriOffset = cashTriOffset;}

    inline float getArea() const {return m_area;}
    inline float getComX() const {return m_comX;}
    inline float getComY() const {return m_comY;}

    inline float getPivotX() const {return m_pivotX;}
    inline float getPivotY() const {return m_pivotY;}

    inline void setPivot(float x, float y) {m_pivotX = x; m_pivotY = y;}

    static void saveToFile(Polygon *poly, std::ofstream &fs);
    static void loadFromFile(Polygon *poly, std::ifstream &fs);


protected:

    void insertNext(Polygon *p);
    void insertPrev(Polygon *p);

    static void exclude(Polygon *p);

    void updateFirst();

};

}

#endif // POLYGON_H
