#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <fstream>
#include "t2d2.h"
#include "../clipperlib_f/clipper.hpp"
#include "../poly2tri_f/common/shapes.h"

namespace t2d2 {

class SimpPolyData;
class SimpContourData;
class Point;
class Triangle;
class Border;
class BBox;
class GridIndexator;
class Contour;
class PolygonGroup;
class UvProjection;
class UvPathProjecton;


typedef t2d2::Point*                    PointPtr;
typedef t2d2::Contour*                  ContourPtr;
typedef std::vector<t2d2::ContourPtr>   ContourPtrVec;

#define POLY_FLAGS_SIZE 5

class Polygon
{
    friend class PolygonGroup;

    class BorderClipperData {
    public:
        Border *m_border;
        ClipperLib::Path m_contour;
        int m_indices[4];
        BorderClipperData(Border *b) : m_border(b) { m_indices[0] = m_indices[1] = m_indices[2] = m_indices[3] = -1; }

        inline int &oS() {return m_indices[0];}
        inline int &oE() {return m_indices[1];}
        inline int &wS() {return m_indices[2];}
        inline int &wE() {return m_indices[3];}
    };

protected:
    PolygonGroup*           m_polyGroup;

    Contour*                m_outline;
    ContourPtrVec           m_holes;

    GridIndexator*          m_indexator;

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

    Polygon *m_prev;
    Polygon *m_next;

    bool                m_skipValidation;

public:

    Polygon(PolygonGroup *pg);
    ~Polygon();

    Polygon *prev();
    Polygon *next();
    Polygon *findLast();

    Contour *outline();

    int     holesCount();
    Contour* hole(unsigned int index);
    Contour* addHole();
    void    deleteHole(unsigned int index);

    inline GridIndexator * indexator() {return m_indexator;}

    unsigned int triNumber();
    Triangle*  tri(int index);

    bool validate       (bool withHoles);
    void markValid();
    void triangulate    (bool force, bool ignoreValidation);
    void deleteTriangles();

    float            zValue()                   {return m_zValue;}
    unsigned int     subMeshIndex()             {return m_subMeshIndex;}

    void    setZValue(float v)                  {m_zValue = v;}
    void    setSubMeshIndex(unsigned int smi)   {m_subMeshIndex = smi;}

    void updateBBox();
    void updateIndexator(int gridSize);

    BBox *bbox();

    void            createUvProjection(float *matrix);
    UvProjection*   getUvProjection() {return m_uvProjection;}

    void  projectUV();

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

    void setFlags(bool *flags);
    void getFlags(bool *flags);

    static void saveToFile(Polygon *poly, std::ofstream &fs);
    static void loadFromFile(Polygon *poly, std::ifstream &fs);

    void updateArea();
    void updateCOM();


    static t2d2::SimpPolyData *createSimpPolyData(Polygon *poly, float *trMtx);

    bool generateBorders(std::vector<Polygon *> & outBordersPoly);

    void complicate(int level);

protected:

    void insertNext(Polygon *p);
    void insertPrev(Polygon *p);

    static void exclude(Polygon *p);

    void updateFirst();

    bool clipBy(SimpPolyData *spd, std::vector<t2d2::Polygon*> &outPolyVec);


    static void addSubjToClipper(ClipperLib::Clipper &clipper, Polygon*poly);
    static void addClipToClipper(ClipperLib::Clipper &clipper, t2d2::SimpPolyData *spd);
    static void buildPolyVecFromClipperTree(ClipperLib::PolyTree &tree, t2d2::Polygon *basePoly, std::vector<t2d2::Polygon*> &outVec);
    static void copyPolyAttributes(Polygon* dp, Polygon *sp);

    bool generateBordersAtContour(Border *b, unsigned int bmask, Contour *cntr,  float holeFactor, std::vector<Polygon *> &outBordersPoly);

    void generateZeroClipPath(ClipperLib::IntPoint &zO, ClipperLib::IntPoint &zW, bool holeFactor, ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zClPath);
    ClipperLib::IntPoint localizeZeroPoint(ClipperLib::Path &path, int zIndex, ClipperLib::IntPoint &zp2);
    ClipperLib::IntPoint offsetZeroPoint(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float cosA);
    void generateFullBorder (ClipperLib::Path &path, float wDelta, float oDelta, ClipperLib::IntPoint &zeroPoint, BorderClipperData &bcd);
    void generateFullBorder (ClipperLib::Path &path, float wDelta, float oDelta, ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zpClPath, BorderClipperData &bcd);
    int generateBorderSegment(ClipperLib::Path &p, float midDelta, float halfDelta, BorderClipperData &bcd);
    void sliceBcd(BorderClipperData &bcd, ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zpClPath, BorderClipperData &bcdA, BorderClipperData &bcdB);


    void cleanupBCD(BorderClipperData &bcd);
    void buildPolyFromBCD(BorderClipperData &bcd, UvPathProjecton &uvp, bool isHole, std::vector<Polygon *> &outPoly);


    void generateUvPathProjData(UvPathProjecton &uvpd, Contour *cntr, float delta);
    void generateUvPathProjection(UvPathProjecton &uvp, BorderClipperData &bcd, Contour *cntr, bool isHole);
};

}

#endif // POLYGON_H
