#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "poly2tri_f/poly2tri.h"

namespace t2d2 {

struct Point;
class Polygon;
struct BBox;
class PolygonGroup;

struct Point: public p2t::Point {
    int index;
    Point() : p2t::Point(), index(-1) {}
};

typedef std::vector<Point *> Points;


struct BBox {
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    bool adequate;

    BBox() { reset(); }

    void reset() {
        xmin = ymin = 1e38f;
        xmax = ymax = -1e37f;
    }

    void update(const Point *p) {
        if ( p->x < xmin)
            xmin = p->x;
        if (p->x > xmax)
            xmax = p->x;
        if (p->y < ymin)
            ymin = p->y;
        if (p->y > ymax)
            ymax = p->y;
    }
};



class PolygonGroup {
    t2d2::Polygon *m_polygon;
public :
    PolygonGroup() : m_polygon(nullptr) {reset(); }
    ~PolygonGroup() { clean(); }
    Polygon *polygon() {return m_polygon;}
    Polygon *reset();
    void clean();
};

class Polygon
{
    Points              m_contour;
    std::vector<Points> m_holes;
    BBox                m_bbox;

    //TODO: main flags
    bool                m_genMesh;
    bool                m_genCollider;
    bool                m_clippingSubj;
    bool                m_clippingClip;

    //TODO: uv projection system




    Polygon *m_first;
    Polygon *m_prev;
    Polygon *m_next;

public:
    Polygon();
    ~Polygon();
    Polygon *prev();
    Polygon *next();
    Polygon *first();

    void insertNext(Polygon *p);
    void insertPrev(Polygon *p);

    static void exclude(Polygon *p);

    Points &contour();
    Points *ptrContour();

    int holesCount();
    Points &hole(int index);
    Points *ptrHole(int index);
    Points &addHole();
    Points *ptrAddHole();
    void deleteHole (int index);

    static void cleanPoints(Points &points);
    static void cleanPoints(Points &points, int index, int count);

    void updateBBox();
    const BBox &bbox();

protected:
    void updateFirst();

};

}

#endif // POLYGON_H
