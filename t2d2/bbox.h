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

    bool isOk();

    void reset();

    void addPoint(const Point *p);

    bool contains (t2d2::Point *p);
    bool contains (t2d2::BBox *bb);

    void getAveragePoint(float *x, float *y);
};

}
#endif // BBOX_H
