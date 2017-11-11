#include "triangle.h"

#define T_EPSILON CLINT2FLOAT
#define T_EPSILON2 (CLINT2FLOAT*CLINT2FLOAT)

t2d2::Triangle::Triangle()
{
    m_p[0] = m_p[1] = m_p[2] = nullptr;
    m_neighbours[0] = m_neighbours[1] = m_neighbours[2] = nullptr;
}

t2d2::PointPtr t2d2::Triangle::getThirdPoint(t2d2::PointPtr pA, t2d2::PointPtr pB)
{
    if (pA == p0())
        return (pB == p1())? p2() : p1();
    if (pA == p1())
        return (pB == p0())? p2() : p0();
    if (pA == p2())
        return (pB == p1())? p0() : p1();
    return nullptr;
}

bool t2d2::Triangle::contains(t2d2::Triangle *t, t2d2::PointPtr p)
{
//    Log()<<__FUNCTION__<<"tri:"<<*(t->p0())<<*(t->p1())<<*(t->p2())<<"p"<<*p;

    if (!pointInTriangleBoundingBox(t, p))
        return false;

    if (naivePointInTriangle(t, p))
        return true;

    if (distanceSquarePointToSegment(t->p0(), t->p1(), p) <= T_EPSILON2)
        return true;

    if (distanceSquarePointToSegment(t->p1(), t->p2(), p) <= T_EPSILON2)
        return true;

    if (distanceSquarePointToSegment(t->p2(), t->p0(), p) <= T_EPSILON2)
        return true;

    return false;
}

t2d2::Triangle *t2d2::Triangle::findTriangle(t2d2::PointPtr p, TrianglePtr tri, unsigned int triNum)
{
    Triangle *t = tri;
    for(unsigned int i = 0; i < triNum; i++) {
//        Log()<<__FUNCTION__<<"test tri:"<<*(t->p0())<<*(t->p1())<<*(t->p2());
        if (contains(t, p))
                return t;
        t++;
    }
    return nullptr;
}

void t2d2::Triangle::computeUV(t2d2::PointPtr p, t2d2::TrianglePtr tri, unsigned int triNum)
{
    if (tri == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no triangulation data";
        return;
    }

    TrianglePtr baseT = findTriangle(p, tri, triNum);
    if (baseT == nullptr) {
        Log(ltError)<<__FUNCTION__<<"base tri not found: ";
        return;
    }

    float b0, b1, b2;

    getBarycentric(baseT, p, b0, b1, b2);

    PointPtr p0 = baseT->p0();
    PointPtr p1 = baseT->p1();
    PointPtr p2 = baseT->p2();


    p->m_u = p0->m_u * b0 + p1->m_u * b1 + p2->m_u * b2;
    p->m_v = p0->m_v * b0 + p1->m_v * b1 + p2->m_v * b2;

//    Log()<<__FUNCTION__<<"p0"<<*p0<<"p1"<<*p1<<"p2"<<*p2<<"p3"<<*p3;
}

bool t2d2::Triangle::naivePointInTriangle(t2d2::Triangle *t, t2d2::PointPtr p)
{
    bool checkSide1 = (t2d2::util::triOrient(p, t->p0(), t->p1()) >= 0);
    bool checkSide2 = (t2d2::util::triOrient(p, t->p1(), t->p2()) >= 0);
    bool checkSide3 = (t2d2::util::triOrient(p, t->p2(), t->p0()) >= 0);

    return checkSide1 && checkSide2 && checkSide3;
}

bool t2d2::Triangle::pointInTriangleBoundingBox(t2d2::Triangle *t, t2d2::PointPtr p)
{
    PointPtr p0 = t->p0();
    PointPtr p1 = t->p1();
    PointPtr p2 = t->p2();

    float xMin = std::min(p0->x, std::min(p1->x, p2->x)) - T_EPSILON;
    float xMax = std::max(p0->x, std::max(p1->x, p2->x)) + T_EPSILON;
    float yMin = std::min(p0->y, std::min(p1->y, p2->y)) - T_EPSILON;
    float yMax = std::max(p0->y, std::max(p1->y, p2->y)) + T_EPSILON;

    if (p->x < xMin || xMax < p->x || p->y < yMin || yMax < p->y)
        return false;
    else
        return true;
}

float t2d2::Triangle::distanceSquarePointToSegment(t2d2::PointPtr p0, t2d2::PointPtr p1, t2d2::PointPtr p)
{
    float p0_p1_squareLength = t2d2::util::dist2(p1, p0);
    float dotProduct =  ((p->x - p0->x)*(p1->x - p0->x) + (p->y - p0->y)*(p1->y - p0->y)) / p0_p1_squareLength;
    if ( dotProduct < 0 ) {
        return t2d2::util::dist2(p, p0);
    } else if ( dotProduct <= 1.0f ){
        return t2d2::util::dist2(p0, p) - dotProduct * dotProduct * p0_p1_squareLength;
    } else  {
        return t2d2::util::dist2(p1, p);
    }
}

void t2d2::Triangle::getBarycentric(t2d2::TrianglePtr t, t2d2::PointPtr p, float &barryA, float &barryB, float &barryC)
{
    PointPtr a = t->p0();
    PointPtr b = t->p1();
    PointPtr c = t->p2();

    float denominator = (b->y - c->y) * (a->x - c->x) + (c->x - b->x) * (a->y - c->y);

    barryA = ((b->y - c->y) * (p->x - c->x) + (c->x - b->x) * (p->y - c->y)) / denominator;

    barryB = ((c->y - a->y) * (p->x - c->x) + (a->x - c->x) * (p->y - c->y)) / denominator;

    barryC = 1 - barryA - barryB;
}

//void t2d2::Triangle::getQuadrangle(t2d2::TrianglePtr t, t2d2::PointPtr &p0, t2d2::PointPtr &p1, t2d2::PointPtr &p2, t2d2::PointPtr &p3)
//{
//    TrianglePtr n;

//    if (t->n0() != nullptr) {
//        p0 = t->p0();
//        p1 = t->p1();
//        p3 = t->p2();
//        n = t->n0();
//    } else  if (t->n1() != nullptr) {
//        p0 = t->p1();
//        p1 = t->p2();
//        p3 = t->p0();
//        n = t->n1();
//    } else  if (t->n2() != nullptr) {
//        p0 = t->p2();
//        p1 = t->p0();
//        p3 = t->p1();
//        n = t->n2();
//    }

//    p2 = n->getThirdPoint(p1, p3);

//    if (p2 == nullptr) {
//        Log(ltError)<<__FUNCTION__<<"!!!!!!!!!!!!! p2 not found !!!!!!!!!!!!!!!!!";
//        return;
//    }
//}


