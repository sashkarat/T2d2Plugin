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

void t2d2::GridIndexator::pushContour(t2d2::Contour *cntr)
{
    if (cntr->m_data.size() == 0)
        return;
    t2d2::Point *p = dynamic_cast<t2d2::Point*>(cntr->m_data[0]);
    t2d2::Point *ep = p;

    do {
        int index = getIndex (p);
        if (index >= 0) {
            if (m_cells[index] == 0) {
                m_cells[index] = new Cell();
                m_cells[index]->m_p = p;
            } else {
                m_cells[index]->m_lp->m_enp = p;
            }
            m_cells[index]->m_lp = p;
        }
        p = p->m_np;
    } while (p!= ep);
}

t2d2::GridIndexator::GridIndexator(Polygon *poly, int gridSize)
{
    m_epsilon = 0.0001f;

    t2d2::BBox *bbox = poly->bbox();

    if (!bbox->isOk())
        poly->updateBBox();

    float ex = (bbox->xmax - bbox->xmin) / gridSize / 4.0f;
    float ey = (bbox->ymax - bbox->ymin) / gridSize / 4.0f;

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

    pushContour(poly->outline());
    int hc = poly->holesCount();
    for(int hi = 0; hi < hc; hi++) {
        pushContour(poly->hole(hi));
    }
}

t2d2::GridIndexator::~GridIndexator()
{
    for(int i = 0; i < m_size; i++)
        if (m_cells[i] != 0)
            delete m_cells[i];

    delete [] m_cells;
}

t2d2::Point *t2d2::GridIndexator::getPoint(float x, float y)
{
    int index = getIndex(x, y);
    if (index < 0 || index >= m_size)
        return nullptr;

    Cell * c = m_cells[index];

    if (c == 0)
        return 0;


    t2d2::Point *tp = c->m_p;

    while (tp != nullptr) {

        if (t2d2::util::pointsEqual(x, y, tp->x, tp->y, m_epsilon))
            return tp;

        tp = tp->m_enp;
    }
    return nullptr;
}

t2d2::Point *t2d2::GridIndexator::getPoint(t2d2::Point *p)
{
    return getPoint(p->x, p->y);
}
