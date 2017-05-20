#ifndef UTIL_H
#define UTIL_H

#include "contour.h"



namespace t2d2 {

    class Point;
    class Contour;

    namespace util {

        bool almostEqual2sComplement(float a, float b, int maxUlps);

        inline bool lessOrEq(float a, float b)

        {
            return (t2d2::util::almostEqual2sComplement(a, b, 1) || a < b);
        }

        inline bool moreOrEq(float a, float b)
        {
            (t2d2::util::almostEqual2sComplement(a, b, 1) || a > b);
        }

        bool intersects(float *a, float *b, float *c, float *d);

        bool pointToSegmentProjection(float *a, float *b, float *c, float * res);

        bool pointOnSegment(float *a, float *b, float *c);

        bool pointOnContour(float *poly, int length, int stride, float *point);

        bool contourContains(float *poly, int length, int stride, float *point);

        int contourEdgeSelfIntersection(float *poly, int length, int stride, int index);

        int segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC);

        int findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out);


        bool hasContourEdgeSelfIntersection(Contour *contour);

        bool contourContainsSegment(t2d2::Contour *contour, t2d2::Point *pointA, t2d2::Point *pointB, bool segmentIntersectionRule);

        bool isHoleContourValid(t2d2::Contour *holeContour, t2d2::Contour *contour);

        void getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax);

        void getGeometricCenter(float *contour, int length, int stride, float *outX, float *outY);

        float triaArea(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC);

        void triMidPoint(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC, float *outX, float *outY);
    }

}
#endif // UTIL_H
