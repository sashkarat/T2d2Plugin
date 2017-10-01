#ifndef COLLIDERCASH_H
#define COLLIDERCASH_H

#include "t2d2.h"
#include "clipperlib_f/clipper.hpp"


namespace t2d2 {

class Contour;
class Polygon;
class PolygonGroup;

class ColliderCash
{
protected:

    class FixedColliderData {
    public:
        float *m_points;
        unsigned int m_len;
        FixedColliderData(unsigned int size);
        ~FixedColliderData();

        void set(unsigned int index, float x, float y);

        void copy(float *out);
    };

    class ColliderData {
    public:
        Contour *m_contour;

        FixedColliderData *m_fixed;


        ColliderData() : m_contour(nullptr), m_fixed(nullptr){}

        ~ColliderData() {if (m_fixed) delete m_fixed;}

        void setContour(Contour *cntr) {m_contour = cntr;}

        unsigned int len();

        void getPoints(float *out);

        bool isFixed();

        void makeFixed();

        void makeFixed(ClipperLib::Path &path);

        void setAsFixed(float *points, float len);
    };


    unsigned int  m_colliderNum;
    ColliderData *m_data;
    PolygonGroup *m_pg;

    float m_area;
    float m_comX;
    float m_comY;

    void clear();

public:

    ColliderCash(PolygonGroup * pg);

    void build();

    void makeFixed();

    void offset(float o);

    void average(unsigned int wsize, unsigned int step);

    ~ColliderCash();

    inline unsigned int num() {return m_colliderNum;}

    inline unsigned int len(unsigned int index) {
        return m_data[index].len();
    }

    inline void getPoints(unsigned int index, float *out) {
        m_data[index].getPoints(out);
    }

    inline float area() { return m_area;}

    inline void com(float *out) { out[0] = m_comX; out[1] = m_comY;}

    void rebuildFromClipperPolyTree(ClipperLib::PolyTree &pt);
};



}

#endif // COLLIDERCASH_H
