#ifndef POINT_H
#define POINT_H
#include "../poly2tri_f/poly2tri.h"
#include "t2d2.h"

namespace t2d2 {

class Contour;

#define OF_RESTORED  0x01


class Point: public p2t::Point {
public:
    Contour*    m_contour;
    int         m_index;
    int         m_borderFlags;
    float       m_u;
    float       m_v;

    t2d2::Point *m_np;
    t2d2::Point *m_pp;
    t2d2::Point *m_enp;

    Point(Contour *contour, t2d2::Point *pp);

    Point(float x, float y, Contour *contour, t2d2::Point *pp);

    static void saveToFile(t2d2::Point* p, std::ofstream &fs);

    static void loadFromFile(t2d2::Point* p, std::ifstream &fs);

    static void copyAttributes (t2d2::Point* dp, t2d2::Point *sp);

protected:
    inline void initDefault(t2d2::Point *pp) {
        m_index = -1;
        m_borderFlags = 0;
        m_np = nullptr;
        m_pp = nullptr;

        if (pp != nullptr) {

            if (pp->m_np != nullptr) {
                pp->m_np->m_pp = this;
                m_np = pp->m_np;
            }

            pp->m_np = this;
            m_pp = pp;
        }
        m_enp = 0;
    }
};

}
#endif // POINT_H
