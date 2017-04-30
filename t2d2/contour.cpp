#include "contour.h"

using namespace t2d2;



int Contour::getCashOffset() const
{
    return m_cashOffset;
}

void Contour::setCashOffset(int cashOffset)
{
    m_cashOffset = cashOffset;
}

Polygon *Contour::getPoly() const
{
    return m_poly;
}

Contour::Contour(Polygon *poly, bool isContour) :
    m_poly(poly),
    m_isContour(isContour),
    m_cashOffset(-1)
{

}

Contour::~Contour()
{
    for(unsigned int j = 0; j< m_data.size(); j++)
        delete m_data[j];
}

void Contour::clean()
{
    for(unsigned int j = 0; j< m_data.size(); j++)
        delete m_data[j];
    if (m_isContour)
        m_poly->updateBBox();
}

unsigned int Contour::length()
{
    return static_cast<unsigned int>(m_data.size());
}

t2d2::Point *Contour::getPoint(unsigned int index)
{
    if (index >= m_data.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return nullptr;
    }
    return dynamic_cast<t2d2::Point*>(m_data[index]);
}

bool Contour::remove (unsigned int startIndex, unsigned int count)
{
    if (startIndex >= m_data.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return false;
    }

    std::vector<p2t::Point *>::const_iterator is = m_data.begin() + startIndex;
    std::vector<p2t::Point *>::const_iterator ie = m_data.begin() + startIndex + count;
    if (ie > m_data.end())
        ie = m_data.end();
    std::vector<p2t::Point *>::const_iterator i = is;
    while (i != ie) {
        p2t::Point *p = *i;
        delete p;
    }
    m_data.erase(is, ie);
    if (m_isContour)
        m_poly->updateBBox();
    return true;
}

unsigned int Contour::getValue(unsigned int startIndex, unsigned int length, float *out, unsigned int stride, bool fillByZValue)
{
    if ( startIndex >= m_data.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }
    unsigned int l = 0;
    for(unsigned int i= startIndex; l < length && i < m_data.size(); i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[i]);
        out[0] = p->x;
        out[1] = p->y;
        if (fillByZValue) {
            unsigned int st = stride;
            while (st > 2)
                out [st-- - 1] = m_poly->zValue();
        }
        out += stride;
        l++;
    }
    return l;
}

unsigned int Contour::setValue(unsigned int startIndex, float *in, unsigned int length, int stride)
{
    if ( startIndex >= m_data.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }

    unsigned int os = static_cast<unsigned int>(m_data.size());

    if ((startIndex + length) > m_data.size())
        m_data.resize(startIndex + length);

    unsigned int e = startIndex + length;
    for(unsigned int i = startIndex; i < e; i++ ) {
        t2d2::Point *p;
        if (i < os) {
            p = dynamic_cast<t2d2::Point*>(m_data[i]);
        } else {
            p = new t2d2::Point(this);
            m_data[i] = p;
        }
        p->x = in[0];
        p->y = in[1];
        in += stride;
    }
    if (m_isContour)
        m_poly->updateBBox();
    return length;
}

unsigned int Contour::addValue(float *in, unsigned int length, unsigned int stride)
{
    for(unsigned int i = 0; i < length; i++) {
        Point *p = new t2d2::Point(this);
        p->x = in[0];
        p->y = in[1];
        m_data.push_back(p);
        in += stride;
        if (m_isContour)
            m_poly->bbox().update(p);
    }
    return length;
}

