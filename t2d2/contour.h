#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>
#include <fstream>

#include "t2d2.h"
#include "../poly2tri_f/poly2tri.h"



namespace t2d2 {

class BBox;
class PolygonGroup;
class Polygon;
class Contour;
class GridIndexator;

typedef t2d2::Contour*                  ContourPtr;
typedef std::vector<t2d2::ContourPtr>   ContourPtrVec;

class Point: public p2t::Point {
public:
    Contour*    m_contour;
    int         m_index;
    int         m_borderFlags;
    float       m_miterX;
    float       m_miterY;
    float       m_dotPr;
    float       m_normX;
    float       m_normY;

    Point(Contour *contour) :
        p2t::Point(),
        m_contour(contour), m_index(-1), m_borderFlags(0), m_normX(0), m_normY(0), m_miterX(0), m_miterY(0), m_dotPr(0) {}

    Point(float x, float y, Contour *contour) :
        p2t::Point(x, y), m_contour(contour),
        m_index(-1), m_borderFlags(0), m_normX(0), m_normY(0), m_miterX(0), m_miterY(0), m_dotPr(0) {}

    inline bool isBorderGeometryValid() {
        return (*(reinterpret_cast<int *>(&m_dotPr)) != 0);
    }

};

typedef t2d2::Point* PointPtr;

class Contour
{
    friend class Polygon;
    friend class PolygonGroup;
    friend class GridIndexator;
protected:

    Polygon*                    m_poly;
    BBox*                       m_bbox;
    GridIndexator*              m_indexator;
    std::vector<p2t::Point*>    m_data;
    bool                        m_isContour;
    int                         m_cashOffset;
    bool                        m_valid;

    void    updateBBox();


public:

    Contour(Polygon *poly, bool isContour = false);
    ~Contour();

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

    void updateIndexator(int gridSize = 10);
    GridIndexator *indexator();

    void setBorderFlags(int startIndex, int *flags, int length);
    void updateNormal(t2d2::PointPtr p, t2d2::PointPtr next);
    void updateMiter(PointPtr prev, PointPtr p);
    void updateBorderGeometry();
    void getNormals(unsigned int startIndex, int length, float *out);
    void getMiters(unsigned int startIndex, int length, float *out);
    void getDotPrValues(unsigned int startIndex, int length, float *out);

    int getCashOffset() const;
    void setCashOffset(int cashOffset);
    Polygon *getPoly() const;

    void validate();
    bool isValid() const;

    p2t::Point * operator[] (int index);
    BBox *bbox() const;


    static void saveToFile(Contour *c, std::ofstream &fs);

    static void loadFromFile(Contour *c, std::ifstream &fs);
};

}
#endif // CONTOUR_H
