#ifndef UTIL_H
#define UTIL_H

namespace t2d2 {

    bool almostEqual2sComplement(float a, float b, int maxUlps);

    bool intersects(float *a, float *b, float *c, float *d);

    bool pointToSegmentProjection(float *a, float *b, float *c, float * res);

    bool pointOnSegment(float *a, float *b, float *c);

    bool pointInPolygon(float *poly, int length, int stride, float *point);

    int contourEdgeSelfIntersection(float *poly, int length, int stride, int index);

    int segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC);

    int findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out);

    void getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax);
}
#endif // UTIL_H
