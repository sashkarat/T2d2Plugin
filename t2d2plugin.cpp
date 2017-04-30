#include "t2d2plugin.h"

#include "t2d2/t2d2.h"
#include "t2d2/polygon.h"


#define T2D2_VERSION    0

#define _CAST_2POLY_G(x)    (static_cast<t2d2::PolygonGroup*>(x))
#define _CAST_2POLY(x)      (static_cast<t2d2::Polygon*>(x))
#define _CAST_2CONTOUR(x)   (static_cast<t2d2::Contour*>(x))
#define _CAST_2MCASH(x)     (static_cast<t2d2::MCash*>(x))

int t2d2_version()
{
    return T2D2_VERSION;
}

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

//===============================================================

T2d2Hndl t2d2_polygonGroupCreate()
{
    return new t2d2::PolygonGroup();
}

void t2d2_polygonGroupDelete(T2d2Hndl pg)
{
    delete _CAST_2POLY_G(pg);
}

T2d2Hndl t2d2_polygonGroupGetPolygon(T2d2Hndl pg)
{
    return _CAST_2POLY_G(pg)->polygon();
}

T2d2Hndl t2d2_polygonGroupAddPolygon(T2d2Hndl pg)
{
    return _CAST_2POLY_G(pg)->addPolygon();
}

void t2d2_polygonGroupDeletePolygon(T2d2Hndl pg, T2d2Hndl poly)
{
    _CAST_2POLY_G(pg)->deletePolygon(_CAST_2POLY(poly));
}


void t2d2_polygonGroupAllocateMCash(T2d2Hndl pg, int stride, int subMeshNumber)
{
     _CAST_2POLY_G(pg)->allocCash(stride, subMeshNumber);
}


T2d2Hndl t2d2_polygonGroupMeshCash(T2d2Hndl pg)
{
     return _CAST_2POLY_G(pg)->mcash();
}


//===============================================================

T2d2Hndl t2d2_polygonGetFirst(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->first();
}

T2d2Hndl t2d2_polygonGetNext(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->next();
}

T2d2Hndl t2d2_polygonGetPrev(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->prev();
}

T2d2Hndl t2d2_polygonGetContour(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->contour();
}

unsigned int t2d2_polygonGetHolesCount(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->holesCount();
}

T2d2Hndl t2d2_polygonGetHole(T2d2Hndl poly, unsigned int index)
{
    return _CAST_2POLY(poly)->hole (index);
}

T2d2Hndl t2d2_polygonAddHole(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->addHole();
}

void t2d2_polygonDeleteHole(T2d2Hndl poly, unsigned int index)
{
    _CAST_2POLY(poly)->deleteHole(index);
}

void t2d2_polygonUpdateBBox(T2d2Hndl poly)
{
    _CAST_2POLY(poly)->updateBBox();
}

void t2d2_polygonGetBBox(T2d2Hndl poly, float *out, int stride)
{
    const t2d2::BBox &b = _CAST_2POLY(poly)->bbox();
    out[0] = b.xmin;
    out[1] = b.ymin;

    out += stride;

    out[0] = b.xmax;
    out[1] = b.ymax;
}

float t2d2_polygonGetZValue(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->zValue();
}

int t2d2_polygonGetSubMeshIndex(T2d2Hndl poly)
{
    return _CAST_2POLY(poly)->subMeshIndex();
}

void t2d2_polygonSetZValue(T2d2Hndl poly, float zval)
{
    _CAST_2POLY(poly)->setZValue(zval);
}

void t2d2_polygonSetSubMeshIndex(T2d2Hndl poly, int smi)
{
    _CAST_2POLY(poly)->setSubMeshIndex(smi);
}

void t2d2_polygonGetFlags(T2d2Hndl poly, bool *flags)
{
    t2d2::Polygon *p = _CAST_2POLY(poly);
    flags[0] = p->genMesh();
    flags[1] = p->genCollider();
    flags[2] = p->clippingSubj();
    flags[3] = p->clippingClip();
}

void t2d2_polygonSetFlags(T2d2Hndl poly, bool *flags)
{
    t2d2::Polygon *p = _CAST_2POLY(poly);
    p->setGenMesh       (flags[0]);
    p->setGenCollider   (flags[1]);
    p->setClippingSubj  (flags[2]);
    p->setClippingClip  (flags[3]);
}

unsigned int t2d2_contourGetLength(T2d2Hndl cntr)
{
    return _CAST_2CONTOUR(cntr)->length();
}

unsigned int t2d2_contourGetValue(T2d2Hndl cntr, unsigned int startIndex, unsigned int length,
                                 float *out, unsigned int stride, bool fillByZValue)
{
    return _CAST_2CONTOUR(cntr)->getValue(startIndex, length, out, stride, fillByZValue);
}

unsigned int t2d2_contourSetValue(T2d2Hndl cntr, unsigned int startIndex, float *in,
                                 unsigned int length, int stride)
{
    return _CAST_2CONTOUR(cntr)->setValue(startIndex, in, length, stride);
}

bool t2d2_contourRemove(T2d2Hndl cntr, int startIndex, int count)
{
    return _CAST_2CONTOUR(cntr)->remove(startIndex, count);
}

unsigned int t2d2_contourAddValue(T2d2Hndl cntr, float *in, unsigned int length, unsigned int stride)
{
    return _CAST_2CONTOUR(cntr)->addValue(in, length, stride);
}

unsigned int t2d2_mcashStride(T2d2Hndl mcash)
{
    return _CAST_2MCASH(mcash)->stride();
}

unsigned int t2d2_mcashVertexNumber(T2d2Hndl mcash)
{
    return _CAST_2MCASH(mcash)->vertexNumber();
}

unsigned int t2d2_mcashSubMeshNumber(T2d2Hndl mcash)
{
    return _CAST_2MCASH(mcash)->subMeshNumber();
}

unsigned int t2d2_mcashTriangleNumber(T2d2Hndl mcash, unsigned int smi)
{
    return _CAST_2MCASH(mcash)->triangleNumber(smi);
}

void t2d2_mcashGetVertices(T2d2Hndl mcash, float *out)
{
    _CAST_2MCASH(mcash)->cpyVertices(out);
}

void t2d2_mcashGetUv(T2d2Hndl mcash, float *out)
{
    _CAST_2MCASH(mcash)->cpyUv(out);
}

void t2d2_mcashGetIndices(T2d2Hndl mcash, unsigned int smi, int *out)
{
    _CAST_2MCASH(mcash)->cpyIndices(smi, out);
}

bool t2d2_utilContourContains(float *polyPoints, int length, int stride, float *point)
{
    return t2d2::pointInPolygon(polyPoints, length, stride, point);
}

bool t2d2_utilAlmostEquals(float a, float b, int maxUlps)
{
    return t2d2::almostEqual2sComplement(a, b, maxUlps);
}

bool t2d2_utilSegmentIntersect(float *a, float *b, float *c, float *d)
{
    return t2d2::intersects(a, b, c, d);
}

bool t2d2_utilPointToSegmentProjection(float *a, float *b, float *c, float *proj)
{
    return t2d2::pointToSegmentProjection (a, b, c, proj);
}


int t2d2_utilEdgeSelfIntersection(float *points, int length, int stride, int index)
{
    return t2d2::contourEdgeSelfIntersection(points, length, stride, index);
}

int t2d2_utilSegmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC)
{
    return t2d2::segmentContourIntersection(segment, strideS, contour, length, strideC);
}

int t2d2_utilFindNearestEdge(float *polyPoints, int length, int stride, float *point, float *out)
{
    return t2d2::findNearestEdgeToPoint(polyPoints, length, stride, point, out);
}

void t2d2_utilBBox(float *points, int length, int stride, float *outMin, float *outMax)
{
    t2d2::getBoundingBox(points, length, stride, outMin, outMax);
}





