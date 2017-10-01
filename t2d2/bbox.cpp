#include "bbox.h"

using namespace t2d2;

#define bbox_MAX(a, b) ((a<b)?b:a)
#define bbox_MIN(a, b) ((a>b)?b:a)

t2d2::BBox::BBox()
{
    reset();
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
    xmax = bbox_MAX(xmax, p->x);
    xmin = bbox_MIN(xmin, p->x);
    ymax = bbox_MAX(ymax, p->y);
    ymin = bbox_MIN(ymin, p->y);

    xsumm += p->x;
    ysumm += p->y;
    count++;
}

void BBox::addPoint(float x, float y)
{
    xmax = bbox_MAX(xmax, x);
    xmin = bbox_MIN(xmin, x);
    ymax = bbox_MAX(ymax, y);
    ymin = bbox_MIN(ymin, y);

    xsumm += x;
    ysumm += y;
    count++;
}

void BBox::addPointSimp(float x, float y)
{
    xmax = bbox_MAX(xmax, x);
    xmin = bbox_MIN(xmin, x);
    ymax = bbox_MAX(ymax, y);
    ymin = bbox_MIN(ymin, y);
}


bool BBox::contains(Point *p)
{
    return
            t2d2::util::lessOrEq(xmin, p->x) &&
            t2d2::util::lessOrEq(p->x, xmax) &&
            t2d2::util::lessOrEq(ymin, p->y) &&
            t2d2::util::lessOrEq(p->y, ymax);
}

bool BBox::contains(float x, float y)
{
    return
            t2d2::util::lessOrEq(xmin, x) &&
            t2d2::util::lessOrEq(x, xmax) &&
            t2d2::util::lessOrEq(ymin, y) &&
            t2d2::util::lessOrEq(y, ymax);
}

bool BBox::contains(BBox *bb)
{
    return
            t2d2::util::lessOrEq(xmin, bb->xmin) &&
            t2d2::util::lessOrEq(bb->xmax, xmax) &&
            t2d2::util::lessOrEq(ymin, bb->ymin) &&
            t2d2::util::lessOrEq(bb->ymax, ymax);
}

bool BBox::overlap(BBox *bb)
{
    bool a = ((xmin < bb->xmin) && (bb->xmin < xmax));
    bool b = ((xmin < bb->xmax) && (bb->xmax < xmax));
    bool c = ((ymin < bb->ymin) && (bb->ymin < ymax));
    bool d = ((ymin < bb->ymax) && (bb->ymax < ymax));
//    return  (a && c) || (b && d) || (a && d) || (b && c);
    return ((a||b) && (c||d));
}

bool BBox::overlap(BBox *b0, BBox *b1)
{
    if (b0->overlap(b1))
        return true;
    return b1->overlap(b0);
}

void BBox::getAveragePoint(float *x, float *y)
{
    if (count == 0)
        return;

    *x = xsumm / count;
    *y = ysumm / count;
}
