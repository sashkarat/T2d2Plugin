#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>
#include <fstream>

#include "t2d2.h"
#include "../poly2tri_f/poly2tri.h"
#include "../clipperlib_f/clipper.hpp"



namespace t2d2 {

class SimpContourData;
class Point;
class BBox;
class PolygonGroup;
class Polygon;
class Contour;
class GridIndexator;

typedef t2d2::Contour*                  ContourPtr;
typedef std::vector<t2d2::ContourPtr>   ContourPtrVec;



typedef t2d2::Point* PointPtr;

class Contour
{
    friend class Polygon;
    friend class PolygonGroup;
    friend class GridIndexator;
protected:

    Polygon*                    m_poly;
    BBox*                       m_bbox;
    std::vector<p2t::Point*>    m_data;
    bool                        m_isContour;
    int                         m_cashOffset;
    bool                        m_valid;

    float                       m_area;
    float                       m_comX;
    float                       m_comY;

    void    updateBBox();


public:


    Contour(Polygon *poly, bool isContour = false);
    ~Contour();

    inline void closeContour();
    inline void updatePointLinks();

    void clear();
    void clearTriDataRef();

    unsigned int    length();
    t2d2::Point*    getPoint(unsigned int index);
    bool            remove(unsigned int startIndex, unsigned int count);

    unsigned int    getValue(unsigned int startIndex, unsigned int length, float* out, unsigned int stride, bool fillByZValue);
    unsigned int    getValue2d(unsigned int startIndex, unsigned int length, float* out);
    unsigned int    getValue3d(unsigned int startIndex, unsigned int length, float* out);
    unsigned int    setValue(unsigned int startIndex, float *in, unsigned int length, int stride);
    unsigned int    setValue2d(unsigned int startIndex, float *in, unsigned int length);
    unsigned int    setValue3d(unsigned int startIndex, float *in, unsigned int length);
    unsigned int    addValue(float *in, unsigned int length, unsigned int stride);
    unsigned int    addValue2d(float *in, unsigned int length);
    unsigned int    addValue3d(float *in, unsigned int length);

    float   updateArea();
    void    updateCOM();
    float   getArea() const;
    void    getCOM(float *x, float *y);

    void setBorderFlags(int startIndex, int *flags, int length);
    void getBorderFlags(int startIndex, int length, int *out);

    void updatePointPositions();
    void updateBorderGeometry();
    void getNormals(unsigned int startIndex, int length, float *out);
    void getMiters(unsigned int startIndex, int length, float *out);
    void getDotPrValues(unsigned int startIndex, int length, float *out);
    void getPositions(unsigned int startIndex, int length, float *out);

    int getCashOffset() const;
    void setCashOffset(int cashOffset);
    Polygon *getPoly() const;

    void validate();
    bool isValid() const;

    p2t::Point * operator[] (int index);
    BBox *bbox() const;

    void makeClipperLibPath(ClipperLib::Path &path);

    static void makeClipperLibPath(ClipperLib::Path &path, SimpContourData &scd);

    static void makeClipperLibPath(ClipperLib::Path &path, float *in, unsigned int len);

    void setClipperLibPath(ClipperLib::Path &path);

    static void saveToFile(Contour *c, std::ofstream &fs);

    static void loadFromFile(Contour *c, std::ifstream &fs);

    static int restorePointAttributes(Contour *dst, GridIndexator *indexator);

    static void rebuildPointAttributes(Contour *cntr, int lastPrevGenIdx);

    static void calcNormal(Point *p);

    static void calcMiter(Point *p);

    static void calcPointPosition(t2d2::Point *p);
};

}
#endif // CONTOUR_H
