#include "gridindexator.h"

float t2d2::GridIndexator::getEpsilon() const
{
    return m_epsilon;
}

void t2d2::GridIndexator::setEpsilon(float epsilon)
{
    m_epsilon = epsilon;
}

int t2d2::GridIndexator::getIndex(float x, float y)
{
    if (x < m_xMin || x > m_xMax)
        return -1;

    if (y < m_yMin || y > m_yMax)
        return -1;

    int _x = static_cast<int> ((x - m_xMin) / m_dx);
    int _y = static_cast<int> ((y - m_yMin) / m_dy);

    return _y * m_gridWidth + _x;
}

int t2d2::GridIndexator::getIndex(t2d2::Point *p)
{
    return getIndex(p->x, p->y);
}

t2d2::GridIndexator::GridIndexator(Contour *contour, int gridSize)
{
    m_epsilon = 0.00001f;

    t2d2::BBox *bbox = contour->bbox();

    if (!bbox->isOk())
        contour->updateBBox();

    float ex = (bbox->xmax - bbox->xmin) / gridSize / 4;
    float ey = (bbox->ymax - bbox->ymin) / gridSize / 4;

    m_xMin = bbox->xmin - ex;
    m_yMin = bbox->ymin - ey;
    m_xMax = bbox->xmax + ex;
    m_yMax = bbox->ymax + ey;

    m_width = m_xMax - m_xMin;
    m_height = m_yMax - m_yMin;

    float aspect = m_height / m_width;

    if (aspect < 1) {
        m_gridWidth = gridSize;
        m_gridHeight = static_cast<int>(aspect * static_cast<float>(gridSize)) + 1;
    } else {
        m_gridHeight = gridSize;
        m_gridWidth = static_cast<int>(static_cast<float>(gridSize) / aspect) + 1;
    }

    m_dx = m_width / m_gridWidth;
    m_dy = m_height / m_gridHeight;

    m_size = m_gridWidth * m_gridHeight;

    m_cells = new CellPtr[m_size];

    memset((void *)m_cells, 0, sizeof(CellPtr) * m_size);

    for(int i = 0; i < contour->m_data.size(); i++) {
        t2d2::Point * p = dynamic_cast<t2d2::Point*>(contour->m_data[i]);
        int index = getIndex (p);
        if (index < 0)
            continue;
        if (m_cells[index] == nullptr) {
            m_cells[index] = new Cell();
        }
        m_cells[index]->m_points.push_back(p);
    }
}

t2d2::GridIndexator::~GridIndexator()
{
    for(int i = 0; i < m_size; i++)
        if (m_cells[i] != nullptr)
            delete m_cells[i];

    delete [] m_cells;
}

t2d2::Point *t2d2::GridIndexator::getPoint(float x, float y)
{
    int index = getIndex(x, y);
    if (index < 0 || index >= m_size)
        return nullptr;

    Cell * c = m_cells[index];

    for(int i = 0; i < c->m_points.size(); i++) {

        t2d2::Point *tp = c->m_points[i];

        if ( t2d2::util::fastabs(x - tp->x) <= m_epsilon &&
             t2d2::util::fastabs(y - tp->y) <= m_epsilon)
            return tp;
    }

    return nullptr;
}

t2d2::Point *t2d2::GridIndexator::getPoint(t2d2::Point *p)
{
    return getPoint(p->x, p->y);
}
