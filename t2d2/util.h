#ifndef UTIL_H
#define UTIL_H

#include <algorithm>


#include "t2d2.h"
#include "clipperlib_f/clipper.hpp"


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

        inline float orient(float x0, float y0, float x1, float y1, float x2, float y2) {
            return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
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

        float triOrient(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC);

        float triArea(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC);

        void triMidPoint(t2d2::Point *pA, t2d2::Point *pB, t2d2::Point * pC, float *outX, float *outY);

        inline static float dot(float x0, float y0, float x1, float y1) {
            return x0 * x1 + y0 * y1;
        }

        inline float fastabs(float f)
        {
            int i=((*(int*)&f)&0x7fffffff);
            return (*(float*)&i);
        }

        inline void fastnorm(float &nx, float &ny)
        {
            float ax = fastabs(nx);
            float ay = fastabs(ny);

            float ratio = 1.0f / std::max(ax, ay);

            ratio *= (1.29289f - (ax + ay) * ratio * 0.29289f);

            nx *= ratio;
            ny *= ratio;
        }

        inline bool pointsEqual(float x0, float y0, float x1, float y1, float eps)
        {
            return ((fastabs(x1 - x0) <= eps) && (fastabs(y1 - y0) <= eps));
        }

        void avrPoint(float *in, unsigned int sIndex, unsigned int wsize, float &avrX, float &avrY);

        void avrPointOnClosedContour(float *in, unsigned int len, unsigned int sIndex, unsigned int wsize, float &avrX, float &avrY);

        bool averagePolygon(float *in, unsigned int len, float **out, unsigned int &lenOut, unsigned int wsize, unsigned int step);


        inline bool inRange(int val, int a, int b) {
            int mx = std::max(a, b);
            int mn = (mx==a)?b:a;
            return (val >= mn && val <= mx);
        }

        inline bool inClosedRange(int val, int from, int to)
        {
            if (from > to)
                return ((val>= from) || (val <= to));
            return ((val>= from) && (val <= to));
        }

        float dist2(t2d2::Point *pA, t2d2::Point *pB);

        float dist(t2d2::Point *pA, t2d2::Point *pB);

        namespace cl {

            //dot product p0-p1  p2-p3
            inline ClipperLib::cInt dot(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, ClipperLib::IntPoint &p2, ClipperLib::IntPoint &p3) {
                return (p1.X - p0.X) * (p3.X - p2.X) + (p1.Y - p0.Y) * (p3.Y - p2.Y);
            }

            inline float dist(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1) {
                float dx = static_cast<float>(p1.X - p0.X);
                float dy = static_cast<float>(p1.Y - p0.Y);
                return sqrtf(dx * dx + dy * dy);
            }

            inline float dist2(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1) {
                float dx = static_cast<float>(p1.X - p0.X);
                float dy = static_cast<float>(p1.Y - p0.Y);
                return dx * dx + dy * dy;
            }

            inline float cosA(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, ClipperLib::IntPoint &p2, ClipperLib::IntPoint &p3) {
                float d = cl::dist(p0, p1) * cl::dist(p2, p3);
                return cl::dot(p0, p1, p2, p3) / d;
            }

            void normal(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float &nx, float &ny);
            void normalOffsetP0(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float delta, ClipperLib::IntPoint &out);
            void normalOffsetP1(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float delta, ClipperLib::IntPoint &out);
            void miter(ClipperLib::IntPoint &pp, ClipperLib::IntPoint &p, ClipperLib::IntPoint &np, float delta,
                       ClipperLib::IntPoint &outMiter);
            void offsetPoint(ClipperLib::Path &path, size_t index, bool open,
                             float delta, ClipperLib::IntPoint &out);

            void findNearest(ClipperLib::Path &path, ClipperLib::Path &targets, float distTolerance,
                             std::vector<int> &out);
            int findNearest(ClipperLib::Path &path, ClipperLib::IntPoint &target, float distTolerance);
#ifndef ANDROID
            void savePath(ClipperLib::Path &p, const std::string &fileName);
#endif
        }

    }
}
#endif // UTIL_H
