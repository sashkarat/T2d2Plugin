#ifndef BBOX_H
#define BBOX_H

#include "t2d2.h"


namespace t2d2 {

class Point;

class BBox {

public:
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float xsumm;
    float ysumm;
    int   count;


    BBox();

    inline bool isOk() {return (xmin<xmax);}

    void reset();

    void addPoint(const Point *p);
    void addPoint(float x, float y);
    void addPointSimp(float x, float y);

    bool contains (t2d2::Point *p);
    bool contains (float x, float y);
    bool contains (t2d2::BBox *bb);
    bool overlap (t2d2::BBox *bb);

    static bool overlap(t2d2::BBox *b0, t2d2::BBox *b1);

    void getAveragePoint(float *x, float *y);

    inline float width() {return xmax - xmin;}
    inline float height() {return ymax - ymin;}
};

}
#endif // BBOX_H
