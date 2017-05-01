#ifndef UTIL_H
#define UTIL_H

#include "contour.h"



namespace t2d2 {

    class Point;
    class Contour;

    namespace util {


        bool almostEqual2sComplement(float a, float b, int maxUlps);

        bool intersects(float *a, float *b, float *c, float *d);

        bool pointToSegmentProjection(float *a, float *b, float *c, float * res);

        bool pointOnSegment(float *a, float *b, float *c);

        bool pointOnContour(float *poly, int length, int stride, float *point);

        bool contourContains(float *poly, int length, int stride, float *point);

        int contourEdgeSelfIntersection(float *poly, int length, int stride, int index);

        int segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC);

        int findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out);

        void getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax);

        bool pointOnContour(Contour *contour, Point *p);
    }

}
#endif // UTIL_H
