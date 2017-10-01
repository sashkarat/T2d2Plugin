#ifndef GRIDINDEXATOR_H
#define GRIDINDEXATOR_H

#include <vector>
#include "t2d2.h"

namespace t2d2 {

class Contour;
class Polygon;

class GridIndexator
{
    struct Cell {
        std::vector<t2d2::Point *> m_points;
        t2d2::Point *m_p;
        t2d2::Point *m_lp;
    };

    float m_xMin;
    float m_yMin;
    float m_xMax;
    float m_yMax;
    float m_width;
    float m_height;
    float m_dx;
    float m_dy;
    int m_gridWidth;
    int m_gridHeight;

    int m_size;

    typedef Cell* CellPtr;

    CellPtr *m_cells;

    float m_epsilon;

protected:

    int getIndex(float x, float y);
    int getIndex(t2d2::Point *p);

    void pushContour(Contour *cntr);

public:
    GridIndexator(t2d2::Polygon *poly, int gridSize);
    ~GridIndexator();
    t2d2::Point * getPoint(float x, float y);
    t2d2::Point * getPoint(t2d2::Point *p);

    float getEpsilon() const;
    void setEpsilon(float epsilon);
};

}
#endif // GRIDINDEXATOR_H
