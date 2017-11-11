#include "point.h"


#define FILE_POINT_DATA_SIZE 5

t2d2::Point::Point(t2d2::Contour *contour, t2d2::Point *pp):
    p2t::Point(),
    m_contour(contour)
{
    initDefault(pp);
}

t2d2::Point::Point(float x, float y, t2d2::Contour *contour, t2d2::Point *pp):
    p2t::Point(x, y),
    m_contour(contour)
{
    initDefault(pp);
}

void t2d2::Point::saveToFile(t2d2::Point *p, std::ofstream &fs)
{
    static float data[FILE_POINT_DATA_SIZE];

    data[0] = p->x;
    data[1] = p->y;
    data[2] = p->m_u;
    data[3] = p->m_v;
    data[4] = *((float*)(&p->m_borderFlags));

    fs.write((char*)data, sizeof(float) * FILE_POINT_DATA_SIZE);
}

void t2d2::Point::loadFromFile(t2d2::Point *p, std::ifstream &fs)
{
    static float data[FILE_POINT_DATA_SIZE];

    fs.read((char*)data, sizeof(float) * FILE_POINT_DATA_SIZE);

    p->x = data[0];
    p->y = data[1];
    p->m_u = data[2];
    p->m_v = data[3];
    p->m_borderFlags = *((int*)(&data[4]));
}

void t2d2::Point::copyAttributes(t2d2::Point *dp, t2d2::Point *sp)
{
    dp->m_borderFlags = sp->m_borderFlags;
    dp->m_u = sp->m_u;
    dp->m_v = sp->m_v;
}
