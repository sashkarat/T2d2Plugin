#ifndef UTIL_H
#define UTIL_H

#include <algorithm>


#include "t2d2.h"



namespace t2d2 {

    class Point;
    class Contour;

    namespace util {

        inline int _index(int i, int l)
        {
            return (l + (i % l)) % l;
        }

        inline unsigned int _idx(unsigned int index, unsigned int l) {
            return index % l;
        }

        bool almostEqual2sComplement(float a, float b, int maxUlps);

        inline bool lessOrEq(float a, float b)

        {
            return (t2d2::util::almostEqual2sComplement(a, b, 1) || a < b);
        }

        inline bool moreOrEq(float a, float b)
        {
            return (t2d2::util::almostEqual2sComplement(a, b, 1) || a > b);
        }

        bool intersects(float *a, float *b, float *c, float *d);

        bool pointToSegmentProjection(float *a, float *b, float *c, float * res);

        bool pointOnSegment(float *a, float *b, float *c);

        bool pointOnContour(float *poly, int length, int stride, float *point);

        bool contourContains(float *poly, int length, int stride, float *point);

        int contourEdgeSelfIntersection(float *poly, int length, int stride, int index);

        int segmentContourIntersection(float *segment, int strideS, float *contour, int length, int strideC);

        int findNearestEdgeToPoint(float *contour, int length, int stride, float *point, float *out);


        bool hasContourEdgeSelfIntersection(t2d2::Contour *contour);

        bool contourContainsSegment(t2d2::Contour *contour, t2d2::Point *pointA, t2d2::Point *pointB, bool segmentIntersectionRule);

        bool isHoleContourValid(t2d2::Contour *holeContour, t2d2::Contour *contour);

        void getBoundingBox(float *contour, int length, int stride, float *outMin, float *outMax);

        void getAveargePoint(float *contour, int length, int stride, float *outX, float *outY);

        float triaArea(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC);

        void triMidPoint(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC, float *outX, float *outY);


        inline float fastabs(float f)
        {
            int i=((*(int*)&f)&0x7fffffff);
            return (*(float*)&i);
        }

        inline void fastnorm(float &nx, float & ny)
        {
            float ax = fastabs(nx);
            float ay = fastabs(ny);

            float ratio = 1.0f / std::max(ax, ay);

            ratio *= (1.29289f - (ax + ay) * ratio * 0.29289f);

            nx *= ratio;
            ny *= ratio;
        }

        inline float dot(float x0, float y0, float x1, float y1)
        {
            return x0 * x1 + y0 * y1;
        }

        inline bool pointsEqual(float x0, float y0, float x1, float y1, float eps)
        {
            return ((fastabs(x1 - x0) <= eps) && (fastabs(y1 - y0) <= eps));
        }

        void avrPoint(float *in, unsigned int sIndex, float wsize, float &avrX, float &avrY);

        void avrPointOnClosedContour(float *in, unsigned int len, unsigned int sIndex, float wsize, float &avrX, float &avrY);

        bool averagePolygon(float *in, unsigned int len, float **out, unsigned int &lenOut, unsigned int wsize, unsigned int step);
    }
}
#endif // UTIL_H
