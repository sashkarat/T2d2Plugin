#include <algorithm>
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

void Contour::validate()
{
    if (m_isContour)
        m_valid = !t2d2::util::hasContourEdgeSelfIntersection(this);
     else
        m_valid = t2d2::util::isHoleContourValid(this, m_poly->outline());
//    if (m_isContour)
//        Log()<<__FUNCTION__<<"contour validation res:"<<m_valid;
//    else
//        Log()<<__FUNCTION__<<"hole validation res:"<<m_valid;
}

bool Contour::isValid() const
{
    return m_valid;
}

p2t::Point *Contour::operator[](int index)
{
    index = t2d2::util::_index(index, static_cast<int>(m_data.size()));
    return m_data[index];
}

BBox *Contour::bbox() const
{
    return m_bbox;
}

void Contour::saveToFile(Contour *c, std::ofstream &fs)
{
    int s = static_cast<int>(c->m_data.size());

    fs.write((char *)&s, sizeof(int));

    static float data[7];

    for(int i =0; i < c->m_data.size(); i++) {

        t2d2::Point *p = dynamic_cast<t2d2::Point*>(c->m_data[i]);

        data[0] = p->x;
        data[1] = p->y;
        data[2] = p->m_normX;
        data[3] = p->m_normY;
        data[4] = p->m_miterX;
        data[5] = p->m_miterY;
        data[6] = p->m_dotPr;

        fs.write((char*)data, sizeof(float) * 7);
        fs.write((char*)(&(p->m_borderFlags)), sizeof(int));
    }
}

void Contour::loadFromFile(Contour *c, std::ifstream &fs)
{
    c->m_bbox->reset();
    c->m_data.clear();
    int s = 0;


    fs.read((char *)&s, sizeof(int));

    static float data[7];

    for (int i = 0; i < s; i++) {

        fs.read((char*)data, sizeof(float)*7);

        t2d2::Point *p = new t2d2::Point(c);

        p->x = data[0];
        p->y = data[1];
        p->m_normX = data[2];
        p->m_normY = data[3];
        p->m_miterX = data[4];
        p->m_miterY = data[5];
        p->m_dotPr = data[6];

        fs.read((char*)(&(p->m_borderFlags)), sizeof(int));

        c->m_data.push_back(p);
        c->m_bbox->addPoint(p);
    }
}

void Contour::updateBBox()
{
    m_bbox->reset();
    for(int i = 0; i < m_data.size(); i++)
        m_bbox->addPoint(dynamic_cast<t2d2::Point*>(m_data[i]));
}

Contour::Contour(Polygon *poly, bool isContour) :
    m_poly(poly),
    m_isContour(isContour),
    m_cashOffset(-1),
    m_valid(false),
    m_indexator(nullptr),
    m_area(0.0f),
    m_comX(0), m_comY(0)
{
    m_bbox = new BBox();
}

Contour::~Contour()
{
    if (m_indexator != nullptr)
        delete m_indexator;

    delete m_bbox;

    for(unsigned int j = 0; j< m_data.size(); j++)
        delete m_data[j];
}

void Contour::clear()
{
    for(unsigned int j = 0; j< m_data.size(); j++)
        delete m_data[j];
    m_data.resize(0);
    m_bbox->reset();
}

void Contour::clearTriDataRef()
{
    for(unsigned int j = 0; j< m_data.size(); j++)
        m_data[j]->edge_list.resize(0);
}

unsigned int Contour::length()
{
    return static_cast<unsigned int>(m_data.size());
}

t2d2::Point *Contour::getPoint(unsigned int index)
{
//    if (index >= m_data.size()) {
//        Log(ltWarning)<<__FUNCTION__<<"index out of range";
//        return nullptr;
//    }
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

    updateBBox();

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

        if (fillByZValue && stride >= 3) {
            out[2] = m_poly->zValue();
        }

        out += stride;
        l++;
    }
    return l;
}

unsigned int Contour::getValue2d(unsigned int startIndex, unsigned int length, float *out)
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
        out += 2;
        l++;
    }
    return l;
}

unsigned int Contour::getValue3d(unsigned int startIndex, unsigned int length, float *out)
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
        out[2] = m_poly->zValue();
        out += 3;
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

    updateBBox();

    return length;
}

unsigned int Contour::setValue2d(unsigned int startIndex, float *in, unsigned int length)
{
    return setValue(startIndex, in, length, 2);
}

unsigned int Contour::setValue3d(unsigned int startIndex, float *in, unsigned int length)
{
    return setValue(startIndex, in, length, 3);
}

unsigned int Contour::addValue(float *in, unsigned int length, unsigned int stride)
{
    for(unsigned int i = 0; i < length; i++) {
        Point *p = new t2d2::Point(this);
        p->x = in[0];
        p->y = in[1];
        m_data.push_back(p);
        in += stride;

        m_bbox->addPoint(p);

    }

    Log()<<__FUNCTION__<<"data size: "<<m_data.size();

    return length;
}

unsigned int Contour::addValue2d(float *in, unsigned int length)
{
    return addValue(in, length, 2);
}

unsigned int Contour::addValue3d(float *in, unsigned int length)
{
    return addValue(in, length, 3);
}

void Contour::updateIndexator(int gridSize)
{
    if (m_indexator != nullptr)
        delete m_indexator;

    m_indexator = new GridIndexator(this, gridSize);
}

GridIndexator *Contour::indexator()
{
    return m_indexator;
}

float Contour::updateArea()
{
    m_area = 0;
    int c = static_cast<int>(m_data.size()) - 1;
    if (c < 2)
        return m_area;


    //using a loop whith c == data.size - 1 to avoid %c operation
    for( int i = 1; i < c; ++i ) {
        p2t::Point *pi  = m_data[i];
        p2t::Point *pi1 = m_data[i+1];
        p2t::Point *pim1 = m_data[i-1];

        m_area += pi->x * (pi1->y - pim1->y);
    }

    {
        p2t::Point *pi  = m_data[c];
        p2t::Point *pi1 = m_data[0];
        p2t::Point *pim1 = m_data[c-1];

        m_area += pi->x * (pi1->y - pim1->y);
    }

    {
        p2t::Point *pi  = m_data[0];
        p2t::Point *pi1 = m_data[1];
        p2t::Point *pim1 = m_data[c];

        m_area += pi->x * (pi1->y - pim1->y);
    }

    m_area /= 2;

    return m_area;
}

void Contour::updateCOM()
{
    m_comX = m_comY = 0;
    int c = static_cast<int>(m_data.size());

    if (c < 3)
        return;

    float signedArea = 0;

    for(int i = 0; i < c-1; i++) {
        p2t::Point *p0 = m_data[i];
        p2t::Point *p1 = m_data[i+1];

        float sai = (p0->x * p1->y) - (p1->x * p0->y);
        signedArea += sai;

        m_comX += (p0->x + p1->x) * sai;
        m_comY += (p0->y + p1->y) * sai;
    }

    {
        p2t::Point *p0 = m_data[c-1];
        p2t::Point *p1 = m_data[0];

        float sai = (p0->x * p1->y) - (p1->x * p0->y);
        signedArea += sai;

        m_comX += (p0->x + p1->x) * sai;
        m_comY += (p0->y + p1->y) * sai;
    }

    m_comX /= (3.0f * signedArea);
    m_comY /= (3.0f * signedArea);
}

float Contour::getArea() const
{
    return m_area;
}

void Contour::getCOM(float *x, float *y)
{
    *x = m_comX;
    *y = m_comY;
}

void Contour::setBorderFlags(int startIndex, int *flags, int length)
{
    Log()<<__FUNCTION__<<" startIndex: "<<startIndex<<" length: "<<length<<" data size: "<<m_data.size();
    if ((startIndex + length) > m_data.size()) {
        length = static_cast<int>(m_data.size()) - startIndex;
        if (length < 0)
            return;
    }

    int e = startIndex + length;

    for(int i = startIndex; i < e; i++)
        dynamic_cast<t2d2::Point*>(m_data[i])->m_borderFlags = *flags++;
}

void Contour::updatePointPositions()
{
    t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[0]);
    p->m_position = 0.0f;
    float prevX = p->x;
    float prevY = p->y;
    float prevP = p->m_position;


    unsigned int c = m_data.size();

    for(unsigned int i = 1; i < c; i++) {
        p = dynamic_cast<t2d2::Point*>(m_data[i]);
        float dx = p->x - prevX;
        float dy = p->y - prevY;
        prevX = p->x;
        prevY = p->y;

        p->m_position = sqrtf(dx * dx + dy * dy) + prevP;

        prevP = p->m_position;
        Log()<<__FUNCTION__<<"i: "<<i<<" position: "<<p->m_position;
    }

}

void Contour::updateNormal(t2d2::PointPtr p, t2d2::PointPtr next)
{
    float dx = next->x - p->x;
    float dy = next->y - p->y;

    p->m_normX = dy;
    p->m_normY = -dx;

    t2d2::util::fastnorm(p->m_normX, p->m_normY);
}

void Contour::updateMiter(t2d2::PointPtr prev, t2d2::PointPtr p)
{
    float nx0 = prev->m_normX;
    float ny0 = prev->m_normY;

    float nx1 = p->m_normX;
    float ny1 = p->m_normY;

    p->m_miterX = nx0 + nx1;
    p->m_miterY = ny0 + ny1;

    t2d2::util::fastnorm(p->m_miterX, p->m_miterY);

    p->m_dotPr = t2d2::util::dot(p->m_miterX, p->m_miterY, prev->m_normX, prev->m_normY);
}

void Contour::updateBorderGeometry()
{
    t2d2::PointPtr prev = dynamic_cast<t2d2::PointPtr>((*this)[-1]);
    t2d2::PointPtr p    = dynamic_cast<t2d2::PointPtr>((*this)[0]);
    t2d2::PointPtr next = dynamic_cast<t2d2::PointPtr>((*this)[1]);

    updateNormal(prev, p);

    for(int i = 0; i < m_data.size(); i++ ) {
        prev = dynamic_cast<t2d2::PointPtr>((*this)[i - 1]);
        p    = dynamic_cast<t2d2::PointPtr>((*this)[i]);
        next = dynamic_cast<t2d2::PointPtr>((*this)[i + 1]);

        updateNormal(p, next);
        updateMiter(prev, p);
    }

    updatePointPositions();
}

void Contour::getNormals(unsigned int startIndex, int length, float *out)
{
    for(int i = startIndex; i < length; i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>((*this)[i]);
        out[0] = p->m_normX;
        out[1] = p->m_normY;
        out += 2;
    }
}

void Contour::getMiters(unsigned int startIndex, int length, float *out)
{
    for(int i = startIndex; i < length; i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>((*this)[i]);
        out[0] = p->m_miterX;
        out[1] = p->m_miterY;
        out += 2;
    }
}

void Contour::getDotPrValues(unsigned int startIndex, int length, float *out)
{
    for(int i = startIndex; i < length; i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>((*this)[i]);
        *out = p->m_dotPr;
        out++;
    }
}

void Contour::getPositions(unsigned int startIndex, int length, float *out)
{
    for(int i = startIndex; i < length; i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>((*this)[i]);
        *out = p->m_position;
        out++;
    }
}


