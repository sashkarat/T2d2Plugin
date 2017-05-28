#include "bbox.h"

using namespace t2d2;

BBox::BBox()
{
    reset();
}

bool BBox::isOk()
{
    if (xmax < xmin)
        return false;
    return true;
}

void BBox::reset()
{
    xmin = ymin = 1e38f;
    xmax = ymax = -1e37f;
    xsumm = ysumm = 0.0f;
    count = 0;
}

void BBox::addPoint(const Point *p)
{
    if ( p->x < xmin)
        xmin = p->x;
    if (p->x > xmax)
        xmax = p->x;
    if (p->y < ymin)
        ymin = p->y;
    if (p->y > ymax)
        ymax = p->y;

    xsumm += p->x;
    ysumm += p->y;
    count++;
}

bool BBox::contains(Point *p)
{
    return
            t2d2::util::lessOrEq(xmin, p->x) &&
            t2d2::util::lessOrEq(p->x, xmax) &&
            t2d2::util::lessOrEq(ymin, p->y) &&
            t2d2::util::lessOrEq(p->y, ymax);
}

bool BBox::contains(BBox *bb)
{
    return
            t2d2::util::lessOrEq(xmin, bb->xmin) &&
            t2d2::util::lessOrEq(bb->xmax, xmax) &&
            t2d2::util::lessOrEq(ymin, bb->ymin) &&
            t2d2::util::lessOrEq(bb->ymax, ymax);
}

void BBox::getAveragePoint(float *x, float *y)
{
    if (count == 0)
        return;

    *x = xsumm / count;
    *y = ysumm / count;
}
