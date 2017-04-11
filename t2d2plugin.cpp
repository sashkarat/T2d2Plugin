#include "t2d2plugin.h"

#include "t2d2/log.h"
#include "t2d2/polygon.h"

void t2d2_setLogCallback(LogCallback lcb)
{
    t2d2::Log::setCallback(lcb);
}

void t2d2_echoLog(const char *szStr)
{
    t2d2::Log()<<szStr;
}

int t2d2_getAnswer()
{
    return 42;
}

void *t2d2_polygonGroupCreate()
{
    return new t2d2::PolygonGroup();
}

void t2d2_polygonGroupDelete(void *pg)
{
    t2d2::PolygonGroup *pgroup = static_cast<t2d2::PolygonGroup *>(pg);
    delete pgroup;
}

void *t2d2_polygonGroupGetPolygon(void *pg)
{
    t2d2::PolygonGroup *pgroup = static_cast<t2d2::PolygonGroup *>(pg);
    return pgroup->polygon();
}

void *t2d2_polygonGetFirst(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->first();
}

void *t2d2_polygonGetNext(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->next();
}

void *t2d2_polygonGetPrev(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->prev();
}

void t2d2_polygonDelete(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    t2d2::Polygon::exclude(poly);
    delete poly;
}

void *t2d2_polygonAddNew(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    poly->insertNext(new t2d2::Polygon());
    return poly->next();
}

void *t2d2_polygonGetContour(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->ptrContour();
}

int t2d2_polygonGetHolesCount(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->holesCount();
}

void *t2d2_polygonGetHole(void *p, int index)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->ptrHole (index);
}

void *t2d2_polygonAddHole(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    return poly->ptrAddHole();
}

void t2d2_polygonDeleteHole(void *p, int index)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    poly->deleteHole(index);
}

void t2d2_polygonUpdateBBox(void *p)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    poly->updateBBox();
}

void t2d2_polygonGetBBox(void *p, float *pXMin, float *pXMax, float *pYMin, float *pYMax)
{
    t2d2::Polygon *poly = static_cast<t2d2::Polygon *>(p);
    const t2d2::BBox &b = poly->bbox();
    *pXMax = b.xmax;
    *pXMin = b.xmin;
    *pYMin = b.ymin;
    *pYMax = b.ymax;
}

int t2d2_pointsGetCount(void *p)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);
    return static_cast<int>(points.size());
}

void t2d2_pointsResize(void *p, int size)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);
    if (size < points.size()) {
        t2d2::Polygon::cleanPoints(points, size, static_cast<int>(points.size())-size);
    }
    points.resize(size);
}

void t2d2_pointsGetValueVector2(void *p, int startIndex, int count, float *out)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);
    for(int i = 0; i < count; i++){
        t2d2::Point *pnt = points[i + startIndex];
        out[i*2 + 0] = pnt->x;
        out[i*2 + 1] = pnt->y;
    }
}

void t2d2_pointsSetValueVector2(void *p, int startIndex, float *in, int count)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);

    if ((startIndex + count) >= points.size())
        points.resize(startIndex + count);
    for(int i = 0; i < count; i++) {
        t2d2::Point *pnt = points[i+startIndex];
        if (pnt == nullptr) {
            pnt = new t2d2::Point();
            points[i+startIndex] = pnt;
        }
        pnt->x = in[i*2 + 0];
        pnt->y = in[i*2 + 1];
    }
}

void t2d2_pointsRemove(void *p, int startIndex, int count)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);
    t2d2::Polygon::cleanPoints(points, startIndex, count);
    points.erase(points.begin() + startIndex, points.begin() + startIndex + count);
}

void t2d2_pointsAddValueVector2(void *p, float *in, int count)
{
    t2d2::Points &points = *static_cast<t2d2::Points *>(p);
    for(int i = 0; i < count; i++) {
        t2d2::Point *pnt = new t2d2::Point();
        pnt->x = in[i*2 + 0];
        pnt->y = in[i*2 + 1];
        points.push_back(pnt);
    }
}

