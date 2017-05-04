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

    BBox();

    void reset();

    void update(const Point *p);

    bool contains (t2d2::Point *p);
    bool contains (t2d2::BBox *bb);
};

}
#endif // BBOX_H
