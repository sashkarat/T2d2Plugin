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

typedef t2d2::Contour*                  ContourPtr;
typedef std::vector<t2d2::ContourPtr>   ContourPtrVec;

class Point: public p2t::Point {
public:
    Contour*    m_points;
    int         m_index;
    Point(Contour *points) : p2t::Point(), m_points(points), m_index(-1) {}
    Point(float x, float y, Contour *points) : p2t::Point(x, y), m_points(points), m_index(-1) {}
};


class Contour
{
    friend class Polygon;
    friend class PolygonGroup;
protected:

    Polygon*                    m_poly;
    BBox*                       m_bbox;
    std::vector<p2t::Point*>    m_data;
    bool                        m_isContour;
    int                         m_cashOffset;
    bool                        m_valid;

    void    updateBBox();

public:

    Contour(Polygon *poly, bool isContour = false);
    ~Contour();

    void clean();

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

    int getCashOffset() const;
    void setCashOffset(int cashOffset);
    Polygon *getPoly() const;

    void validate();
    bool isValid() const;

    p2t::Point * operator[] (unsigned int index);
    BBox *bbox() const;

    static void saveToFile(Contour *c, std::ofstream &fs);

    static void loadFromFile(Contour *c, std::ifstream &fs);
};

}
#endif // CONTOUR_H
