#include "uvprojection.h"

t2d2::UvProjection::UvProjection()
{
    memset(m_mtx, 0, sizeof(float) * 16);
    m_mtx[0] = m_mtx[5] = m_mtx[10] = m_mtx[15] = 1;

}

void t2d2::UvProjection::setMatrix4x4(float *data)
{
    memcpy(m_mtx, data, sizeof(float) * 16);

//    Log()<<__FUNCTION__<<"<0>"<<m_mtx[0]<<"<4>"<<m_mtx[4]<<"<8>"<<m_mtx[8]<<"<12>"<<m_mtx[12];
//    Log()<<__FUNCTION__<<"<1>"<<m_mtx[1]<<"<5>"<<m_mtx[5]<<"<9>"<<m_mtx[9]<<"<13>"<<m_mtx[13];
}

void t2d2::UvProjection::computeUV(t2d2::Point *p, float &u, float &v)
{
    u = m_mtx[0] * p->x + m_mtx[4] * p->y + m_mtx[8] + m_mtx[12];
    v = m_mtx[1] * p->x + m_mtx[5] * p->y + m_mtx[9] + m_mtx[13];


}


