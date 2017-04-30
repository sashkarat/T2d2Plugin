#include "bbox.h"

using namespace t2d2;

BBox::BBox()
{
     reset();
}

void BBox::reset()
{
    xmin = ymin = 1e38f;
    xmax = ymax = -1e37f;
}

void BBox::update(const Point *p)
{
    if ( p->x < xmin)
        xmin = p->x;
    if (p->x > xmax)
        xmax = p->x;
    if (p->y < ymin)
        ymin = p->y;
    if (p->y > ymax)
        ymax = p->y;
}
