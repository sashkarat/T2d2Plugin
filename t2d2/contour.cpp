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

void Contour::makeClipperLibPath(ClipperLib::Path &path)
{
    size_t pc = m_data.size();
    for(size_t i = 0; i < pc; i++) {
        p2t::Point *p = m_data[i];
        
        path.push_back( ClipperLib::IntPoint(
                            static_cast<ClipperLib::cInt>(p->x * FLOAT2CLINT),
                            static_cast<ClipperLib::cInt>(p->y * FLOAT2CLINT)));
    }
}

void Contour::makeClipperLibPath(ClipperLib::Path &path, t2d2::SimpContourData &scd)
{
    for(int i = 0; i < scd.m_len; i++) {
        int idx = i*2;
        float x = scd.m_points[idx] * FLOAT2CLINT;
        float y = scd.m_points[idx+1] * FLOAT2CLINT;

        path.push_back( ClipperLib::IntPoint(
                            static_cast<ClipperLib::cInt>(x),
                            static_cast<ClipperLib::cInt>(y)));
    }
}

void Contour::makeClipperLibPath(ClipperLib::Path &path, float *in, unsigned int len)
{
    for(int i = 0; i < len; i++) {
        int idx = i*2;
        float x = in[idx] * FLOAT2CLINT;
        float y = in[idx+1] * FLOAT2CLINT;

        path.push_back( ClipperLib::IntPoint(
                            static_cast<ClipperLib::cInt>(x),
                            static_cast<ClipperLib::cInt>(y)));
    }
}

void Contour::setClipperLibPath(ClipperLib::Path &path)
{
    size_t pc = path.size();
    size_t ds = m_data.size();
    for(size_t j = 0; j < ds; j++)
        delete m_data[j];
    m_data.clear();

    t2d2::Point *pp = 0;
    for(int i = 0; i < pc; i++) {

        ClipperLib::IntPoint ip = path[i];

        t2d2::Point *p = new t2d2::Point(
                    ip.X * CLINT2FLOAT,
                    ip.Y * CLINT2FLOAT,
                    this, pp);
        pp = p;
        m_data.push_back(p);
    }
    closeContour();
}

void Contour::saveToFile(Contour *c, std::ofstream &fs)
{
    int s = static_cast<int>(c->m_data.size());

    fs.write((char *)&s, sizeof(int));

    for(int i =0; i < c->m_data.size(); i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>(c->m_data[i]);
        t2d2::Point::saveToFile(p, fs);
    }
}

void Contour::loadFromFile(Contour *c, std::ifstream &fs)
{
    c->m_bbox->reset();
    c->m_data.clear();
    int s = 0;

    fs.read((char *)&s, sizeof(int));
    t2d2::Point *pp = 0;
    for (int i = 0; i < s; i++) {
        t2d2::Point *p = new t2d2::Point(c, pp);
        pp = p;
        t2d2::Point::loadFromFile(p, fs);
        c->m_data.push_back(p);
        c->m_bbox->addPoint(p);
    }
    c->closeContour();
}

int Contour::restorePointAttributes(Contour *dst, GridIndexator *indexator)
{
    size_t ds = dst->m_data.size();
    int lastPrevGenIdx = -1;

    for(size_t i = 0; i < ds; i++) {
        t2d2::Point * dp = dynamic_cast<t2d2::Point*>(dst->m_data[i]);
        t2d2::Point * sp = indexator->getPoint(dp);

        if (sp != 0 ) {
            t2d2::Point::copyAttributes(dp, sp);
            dp->setOF_Gen();
            lastPrevGenIdx = static_cast<int>(i);
//            Log()<<__FUNCTION__<<"OLD POINT: "<<dp->x<<" "<<dp->y;
        } else {
            //TODO: new attributes
//            Log()<<__FUNCTION__<<"NEW POINT: "<<dp->x<<" "<<dp->y;
        }
    }
    return lastPrevGenIdx;
}

void Contour::rebuildPointAttributes(Contour *cntr, int lastPrevGenIdx)
{
    if (cntr->m_data.size() == 0)
        return;

    t2d2::Point *p =  (lastPrevGenIdx < 0)?
                dynamic_cast<t2d2::Point*>(cntr->m_data[0]) :
        dynamic_cast<t2d2::Point*>(cntr->m_data[(lastPrevGenIdx)])->m_np;

    t2d2::Point *ep = p;

    do {
        if (!p->of_Gen()) {
            calcPointPosition(p);
            calcNormal(p);
            calcMiter(p);

            if (p->m_pp->of_Gen())
                p->m_borderFlags = p->m_pp->m_borderFlags;

            if (p->m_np->of_Gen())
                p->m_borderFlags = p->m_np->m_borderFlags;
        }

        p = p->m_np;
    } while (ep != p);
}

void Contour::calcNormal(t2d2::Point *p)
{
    t2d2::Point *np = p->m_np;
    float dx = np->x - p->x;
    float dy = np->y - p->y;

    p->m_normX = dy;
    p->m_normY = -dx;

    t2d2::util::fastnorm(p->m_normX, p->m_normY);
}

void Contour::calcMiter(t2d2::Point *p)
{
    t2d2::Point *pp = p->m_pp;
    float nx0 = pp->m_normX;
    float ny0 = pp->m_normY;

    float nx1 = p->m_normX;
    float ny1 = p->m_normY;

    p->m_miterX = nx0 + nx1;
    p->m_miterY = ny0 + ny1;

    t2d2::util::fastnorm(p->m_miterX, p->m_miterY);

    p->m_dotPr = t2d2::util::dot(p->m_miterX, p->m_miterY, pp->m_normX, pp->m_normY);
}

void Contour::calcPointPosition(t2d2::Point *p)
{
    t2d2::Point *pp = p->m_pp;
    float dx = p->x - pp->x;
    float dy = p->y - pp->y;
    p->m_position = sqrtf(dx * dx + dy * dy) + pp->m_position;
}

void Contour::updateBBox()
{
    m_bbox->reset();
    size_t ds = m_data.size();
    for(size_t i = 0; i < ds; i++)
        m_bbox->addPoint(dynamic_cast<t2d2::Point*>(m_data[i]));
}

Contour::Contour(Polygon *poly, bool isContour) :
    m_poly(poly),
    m_isContour(isContour),
    m_cashOffset(-1),
    m_valid(false),
    m_area(0.0f),
    m_comX(0), m_comY(0)
{
    m_bbox = new BBox();
}

Contour::~Contour()
{
    delete m_bbox;

    for(unsigned int j = 0; j< m_data.size(); j++)
        delete m_data[j];
}

void Contour::closeContour()
{
    size_t ds = m_data.size();
    if (ds == 0)
        return;
    t2d2::Point *lp = dynamic_cast<PointPtr>(m_data[ds-1]);
    t2d2::Point *fp = dynamic_cast<PointPtr>(m_data[0]);
    lp->m_np = fp;
    fp->m_pp = lp;
}

void Contour::updatePointLinks()
{
    size_t ds = m_data.size();
    t2d2::Point *pp = dynamic_cast<PointPtr>(m_data[0]);
    for(size_t i = 1; i < ds; i++) {
        t2d2::Point *p = dynamic_cast<PointPtr>(m_data[i]);
        pp->m_np = p;
        p->m_pp = pp;
        pp = p;
    }
    closeContour();
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
//        return 0;
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
    size_t l = 0;
    size_t ds = m_data.size();
    bool fillZ = fillByZValue && (stride >= 3);

    for(size_t i = startIndex; l < length && i < ds; i++) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[i]);
        out[0] = p->x;
        out[1] = p->y;

        if (fillZ)
            out[2] = m_poly->zValue();

        out += stride;
        l++;
    }
    return static_cast<unsigned int>(l);
}

unsigned int Contour::getValue2d(unsigned int startIndex, unsigned int length, float *out)
{
    unsigned int ds = static_cast<unsigned int>(m_data.size());

    if ( startIndex >= ds) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }
    unsigned int l = 0;
    for(unsigned int i= startIndex; l < length && i < ds; i++) {
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

    unsigned int ds = static_cast<unsigned int>(m_data.size());

    if ((startIndex + length) > m_data.size())
        m_data.resize(startIndex + length);

    unsigned int e = startIndex + length;

    for(unsigned int i = startIndex; i < e; i++ ) {
        t2d2::Point *p;
        if (i < ds) {
            p = dynamic_cast<t2d2::Point*>(m_data[i]);
        } else {
            p = new t2d2::Point(this, 0);
            m_data[i] = p;
        }
        p->x = in[0];
        p->y = in[1];
        in += stride;
    }

    updatePointLinks();

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
    size_t ds = m_data.size();

    t2d2::Point * pp = ( m_data.size() > 0 ) ? dynamic_cast<t2d2::Point*>(m_data[ds-1]) : 0;

    for(unsigned int i = 0; i < length; i++) {
        Point *p = new t2d2::Point(this, pp);
        pp = p;
        p->x = in[0];
        p->y = in[1];
        m_data.push_back(p);
        in += stride;

        m_bbox->addPoint(p);

    }
    closeContour();
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

    m_area = t2d2::util::fastabs(m_area);

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
//    Log()<<__FUNCTION__<<" startIndex: "<<startIndex<<" length: "<<length<<" data size: "<<m_data.size();
    if ((startIndex + length) > m_data.size()) {
        length = static_cast<int>(m_data.size()) - startIndex;
        if (length < 0)
            return;
    }

    int e = startIndex + length;

    for(int i = startIndex; i < e; i++)
        dynamic_cast<t2d2::Point*>(m_data[i])->m_borderFlags = *flags++;
}

void Contour::getBorderFlags(int startIndex, int length, int *out)
{
    if ((startIndex + length) > m_data.size()) {
        length = static_cast<int>(m_data.size()) - startIndex;
        if (length < 0)
            return;
    }

    int e = startIndex + length;

    int *flags = out;

    for(int i = startIndex; i < e; i++)
        *flags++ = dynamic_cast<t2d2::Point*>(m_data[i])->m_borderFlags;
}

void Contour::updatePointPositions()
{
    t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[0]);
    t2d2::Point *ep = p;

    p->m_position = 0.0f;
    p = p->m_np;

    do {
        calcPointPosition(p);
        p = p->m_np;
    } while (ep != p);
}

void Contour::updateBorderGeometry()
{
    t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[0]);
    t2d2::Point *ep = p;

    calcNormal(p->m_pp);

    do {
        calcNormal(p);
        calcMiter(p);

        p = p->m_np;
    } while (ep != p);

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


