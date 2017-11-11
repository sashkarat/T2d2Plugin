#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "t2d2.h"

namespace t2d2 {

class Point;
class Triangle;

typedef t2d2::Point* PointPtr;
typedef t2d2::Triangle* TrianglePtr;

class Triangle
{
public:

    PointPtr m_p[3];
    TrianglePtr m_neighbours[3];

    Triangle();


    inline PointPtr p0() {return m_p[0];}
    inline PointPtr p1() {return m_p[1];}
    inline PointPtr p2() {return m_p[2];}

    inline TrianglePtr n0() {return m_neighbours[0];}
    inline TrianglePtr n1() {return m_neighbours[1];}
    inline TrianglePtr n2() {return m_neighbours[2];}

    PointPtr getThirdPoint(PointPtr pA, PointPtr pB);

    static bool contains(Triangle *t, PointPtr p);
    static Triangle *findTriangle(PointPtr p, TrianglePtr tri, unsigned int triNum);
    static void computeUV(PointPtr p, TrianglePtr tri, unsigned int triNum);



protected:
    static bool naivePointInTriangle(Triangle *t, PointPtr p);
    static bool pointInTriangleBoundingBox(Triangle *t, PointPtr p);
    static float distanceSquarePointToSegment(PointPtr p0, PointPtr p1, PointPtr p);
    static void getBarycentric(TrianglePtr t, PointPtr p, float &b0, float &b1, float &b2);
//    static void getQuadrangle(TrianglePtr t, PointPtr &p0, PointPtr &p1, PointPtr &p2, PointPtr &p3);
};

}

#endif // TRIANGLE_H
