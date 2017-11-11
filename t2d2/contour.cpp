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
    if (m_isOutline)
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

void Contour::complicate(int level)
{
    float maxd2 = -1e35f;

    t2d2::Point *p = getPoint(0);
    t2d2::Point *sp = p;

    do {
        float d2 = t2d2::util::dist2(p, p->m_np);
        if (d2 > maxd2)
            maxd2 = d2;
        p = p->m_np;
    } while (p != sp);

    float delta = sqrtf(maxd2) / level;

//    Log()<<__FUNCTION__<<"maxd2: "<<maxd2<<"delta"<<delta;

    std::vector<p2t::Point*> ndata;

    p = sp;
    do {
        ndata.push_back(p);

        t2d2::Point *onp = p->m_np;

        float d = t2d2::util::dist(p, onp);

        int  cd =  static_cast<int>(d / delta);


        if (cd > 1) {


            float dlt = d / cd;

            float du = (onp->m_u - p->m_u) / cd;
            float dv = (onp->m_v - p->m_v) / cd;

//            Log()<<__FUNCTION__<<"cd"<<cd<<"dlt"<<dlt;

            for(int  i = 1; i < cd; i++) {
                float dx = onp->x - p->x;
                float dy = onp->y - p->y;

                float nx = dx;
                float ny = dy;

                t2d2::util::fastnorm(nx, ny);

                float deltaX = nx * dlt;
                float deltaY = ny * dlt;

                t2d2::Point *newp = new t2d2::Point(this, p);

                newp->x = p->x + deltaX;
                newp->y = p->y + deltaY;


                newp->m_u = p->m_u + du;
                newp->m_v = p->m_v + dv;

                p = newp;

                ndata.push_back(p);
            }
        }

        p = p->m_np;
    } while (p!= sp);

    m_data.clear();
    m_data.insert(m_data.begin(), ndata.begin(), ndata.end());
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

void Contour::restorePointAttributes(Contour *dst, Polygon *basePoly)
{
    GridIndexator *basePolyIndexator = basePoly->indexator();

    Polygon *poly = dst->getPoly();
    UvProjection *prj = poly->getUvProjection();

    size_t ds = dst->m_data.size();

    for(size_t i = 0; i < ds; i++) {
        t2d2::Point * dp = dynamic_cast<t2d2::Point*>(dst->m_data[i]);
        t2d2::Point * sp = basePolyIndexator->getPoint(dp);

        if (sp) {
            t2d2::Point::copyAttributes(dp, sp);
//            Log()<<__FUNCTION__<<"OLD POINT: "<<*dp<<" <- "<<*sp;
        } else {

            if (prj)
                prj->computeUV(dp);
            else
               Triangle::computeUV(dp, basePoly->tri(0), basePoly->triNumber());

//            Log()<<__FUNCTION__<<"NEW POINT: "<<*dp;
        }

    }
}

void Contour::updateBBox()
{
    m_bbox->reset();
    size_t ds = m_data.size();
    for(size_t i = 0; i < ds; i++)
        m_bbox->addPoint(dynamic_cast<t2d2::Point*>(m_data[i]));
}

Contour::Contour(Polygon *poly, bool isOutline) :
    m_poly(poly),
    m_isOutline(isOutline),
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
    size_t ds = m_data.size();
    for(size_t j = 0; j< ds; j++)
        m_data[j]->edge_list.resize(0);
}

unsigned int Contour::length()
{
    return static_cast<unsigned int>(m_data.size());
}

t2d2::Point *Contour::getPoint(size_t index)
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

unsigned int Contour::getUV(unsigned int startIndex, unsigned int length, float *out)
{
    unsigned int ds = static_cast<unsigned int>(m_data.size());

    if ( startIndex >= ds) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }

    if ((startIndex + length) > ds)
        length = ds - startIndex;

    unsigned int e = startIndex + length;

    for(unsigned int i = startIndex; i < e; i++ ) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[i]);
        out[0] = p->m_u;
        out[1] = p->m_v;
        out += 2;
    }
    return length;
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

unsigned int Contour::setUV(unsigned int startIndex, float *in, unsigned int length)
{
    unsigned int ds = static_cast<unsigned int>(m_data.size());

    if ( startIndex >= ds) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }

    if ((startIndex + length) > ds)
        length = ds - startIndex;

    unsigned int e = startIndex + length;

    for(unsigned int i = startIndex; i < e; i++ ) {
        t2d2::Point *p = dynamic_cast<t2d2::Point*>(m_data[i]);
        p->m_u = in[0];
        p->m_v = in[1];
        in += 2;
    }
    return length;
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

void Contour::projectUV(UvProjection *prj)
{
    size_t c = m_data.size();

    for(size_t i = 0; i < c; i++)
        prj->computeUV(dynamic_cast<t2d2::Point*>(m_data[i]));
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
