#ifndef SIMPLEDATA_H
#define SIMPLEDATA_H

#include "t2d2.h"
#include "clipperlib_f/clipper.hpp"


namespace t2d2 {

class Contour;
class BBox;

class SimpContourData {
public:
    ClipperLib::Path m_clPath;

    int m_len = 0;
    float *m_points;
    SimpContourData();
    ~SimpContourData();
    void set(t2d2::Contour *cntr);
    void applyTrMatrix(float *m);
    void createClipperLibPath();
    void alloc4Points();
    void set(int index, float x, float y);

    float getX(int index);
    float getY(int index);
};

class SimpPolyData {
public:
    SimpPolyData *m_next;
    BBox *m_bbox;
    SimpContourData m_outline;
    int m_hc = 0;
    SimpContourData *m_holes;
    SimpPolyData(int hc);
    ~SimpPolyData();
    void applyTrMatrix(float *m);
    void updateBBox();
    void createClipperLibPaths();
};

}

#endif // SIMPLEDATA_H
