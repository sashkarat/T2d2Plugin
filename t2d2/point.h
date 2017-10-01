#ifndef POINT_H
#define POINT_H
#include "../poly2tri_f/poly2tri.h"
#include "t2d2.h"

namespace t2d2 {

class Contour;

#define OF_GEN  0x01


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
    float       m_position;

    int         m_opFlags;
    t2d2::Point *m_np;
    t2d2::Point *m_pp;
    t2d2::Point *m_enp;

    Point(Contour *contour, t2d2::Point *pp);

    Point(float x, float y, Contour *contour, t2d2::Point *pp);

    inline bool isBorderGeometryValid() {
        return (*(reinterpret_cast<int *>(&m_dotPr)) != 0);
    }

    inline int of_Gen() { return m_opFlags & OF_GEN;}
    inline void setOF_Gen() { m_opFlags |= OF_GEN;}
    inline void rstOF_Gen() {m_opFlags &= ~OF_GEN;}

    static void saveToFile(t2d2::Point* p, std::ofstream &fs);

    static void loadFromFile(t2d2::Point* p, std::ifstream &fs);

    static void copyAttributes (t2d2::Point* dp, t2d2::Point *sp);

protected:
    inline void initDefault(t2d2::Point *pp) {
        m_index = -1;
        m_borderFlags = 0;
        m_normX = m_normY = m_miterX = m_miterY = m_dotPr = m_position = 0.0f;
        m_opFlags = 0x00;
        if (pp != 0) {
            pp->m_np = this;
            m_pp = pp;
        }
        m_enp = 0;
    }
};

}
#endif // POINT_H
