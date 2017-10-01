#include "polygon.h"

using namespace t2d2;

void Polygon::saveToFile(Polygon *poly, std::ofstream &fs)
{

    static bool flags[POLY_FLAGS_SIZE];

    poly->getFlags(flags);

    fs.write((char *)flags, sizeof(bool) * POLY_FLAGS_SIZE);


    Contour::saveToFile(poly->m_outline, fs);

    unsigned int s = static_cast<unsigned int>(poly->m_holes.size());

    fs.write((char *)&s, sizeof(int));

    for(unsigned int i = 0; i < s; i++)
        Contour::saveToFile(poly->m_holes[i], fs);
}

void Polygon::loadFromFile(Polygon *poly, std::ifstream &fs)
{
    static bool flags[POLY_FLAGS_SIZE];

    fs.read((char *)flags, sizeof(bool) * POLY_FLAGS_SIZE);

    poly->setFlags(flags);

    Contour::loadFromFile(poly->m_outline, fs);

    int hc  = 0;

    fs.read((char*)&hc, sizeof(int));

    for(int i = 0; i < hc; i++) {
        Contour *hole = new Contour(poly);
        Contour::loadFromFile(hole, fs);
        poly->m_holes.push_back(hole);
    }

    poly->updateIndexator(10);
}

void Polygon::updateArea()
{
    m_area = 0;
    m_area = m_outline->updateArea();
    for(int i = 0; i < m_holes.size(); i++)
        m_area -= m_holes[i]->updateArea();
}

void Polygon::updateCOM()
{
    updateArea();

    m_outline->updateCOM();
    m_outline->getCOM(&m_comX, &m_comY);

    float a = m_outline->getArea();

    m_comX *= a;
    m_comY *= a;

    for(int i = 0; i < m_holes.size(); i++) {
        Contour *hole = m_holes[i];
        float ha = hole->getArea();
        float x, y;
        hole->updateCOM();
        hole->getCOM(&x, &y);

        m_comX -= x * ha;
        m_comY -= y * ha;
        a -= ha;
    }

    m_comX /= a;
    m_comY /= a;
}

t2d2::SimpPolyData *Polygon::createSimpPolyData(Polygon *poly, float *trMtx)
{
    int hc = poly->holesCount();

    t2d2::SimpPolyData *spd = new t2d2::SimpPolyData(hc);

    spd->m_outline.set(poly->m_outline);

    for(int i = 0; i < hc; i++)
        spd->m_holes[i].set(poly->hole(i));

    if (trMtx != 0)
        spd->applyTrMatrix(trMtx);

    spd->updateBBox();

    return spd;
}

Polygon::Polygon(PolygonGroup *pg) : m_prev(0), m_next(0)
{
    m_indexator = 0;

    m_triangles = 0;
    m_triangleNum = 0;

    m_polyGroup = pg;
    m_outline = new Contour(this, true);

    m_uvProjection = new UvProjection();

    m_zValue = 0.0f;
    m_subMeshIndex = 0;
    m_cashTriOffset = -1;

    m_genMesh = true;
    m_genBorders = false;
    m_genCollider = false;
    m_clippingSubj = true;
    m_clippingClip = false;

    m_comX = 0;
    m_comY = 0;
    m_area = 0;
    m_pivotX = 0;
    m_pivotY = 0;
}

Polygon::~Polygon()
{

    if (m_indexator)
        delete m_indexator;

    delete m_uvProjection;

    deleteTriangles();

    delete m_outline;

    for(unsigned int i = 0; i < m_holes.size(); i++)
        delete m_holes[i];
}

Polygon *Polygon::prev()
{
    return m_prev;
}

Polygon *Polygon::next()
{
    return m_next;
}

Polygon *Polygon::findLast()
{
    Polygon *p = this;
    while(p->m_next != 0)
        p = p->m_next;
    return  p;
}

Contour *Polygon::outline()
{
    return m_outline;
}

int Polygon::holesCount()
{
    return static_cast<int>(m_holes.size());
}

Contour *Polygon::hole(unsigned int index)
{
    if (index >= m_holes.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index is out of range";
        return 0;
    }
    return m_holes[index];
}

Contour *Polygon::addHole()
{
    m_holes.push_back( new Contour(this));
    return m_holes [m_holes.size() - 1];
}

void Polygon::deleteHole(unsigned int index)
{
    if (index >= m_holes.size()) {
        Log(ltWarning)<<__FUNCTION__<<"index is out of range";
        return;
    }
    delete m_holes[index];
    m_holes.erase(m_holes.begin()+index);
}

unsigned int Polygon::triNumber()
{
    return m_triangleNum;
}

Polygon::Triangle *Polygon::tri(int index)
{
    return &m_triangles[index];
}

bool Polygon::validate(bool withHoles)
{
    bool res = false;

    m_outline->validate();

    res = m_outline->isValid() || res;

    if (!res)
        return false;

    if (!withHoles)
        return res;

    for(int i = 0; i < m_holes.size(); i++) {
        m_holes[i]->validate();
        res = res || m_holes[i]->isValid();
    }

    return res;
}

void Polygon::markValid()
{
    m_outline->m_valid = true;

    size_t hs = m_holes.size();

    for(size_t i = 0; i < hs; i++) {
        m_holes[i]->m_valid = true;
    }
}

void Polygon::triangulate()
{
    deleteTriangles();

    if (!m_outline->isValid())
        return;

    m_outline->clearTriDataRef();

    p2t::CDT *p2tCdt = new p2t::CDT(m_outline->m_data);

    size_t hs = m_holes.size();

    for(size_t i = 0; i < hs; i++) {
        Contour *holePtr = m_holes[i];

        if (!holePtr->m_valid)
            continue;

        holePtr->clearTriDataRef();

        p2tCdt->AddHole(holePtr->m_data);
    }

    p2tCdt->Triangulate();

    std::vector<p2t::Triangle *> &tri = p2tCdt->GetTriangles();

    m_triangleNum = static_cast<unsigned int>(tri.size());


    m_triangles = new Triangle[m_triangleNum];

    Triangle *pt = m_triangles;


    for(unsigned int i = 0; i < m_triangleNum; i++) {
        p2t::Triangle *st = tri[i];

        PointPtr p0 = dynamic_cast<t2d2::Point*>(st->GetPoint(0));
        PointPtr p1 = dynamic_cast<t2d2::Point*>(st->GetPoint(1));
        PointPtr p2 = dynamic_cast<t2d2::Point*>(st->GetPoint(2));



//        p0->edge_list.resize(0);
//        p1->edge_list.resize(0);
//        p2->edge_list.resize(0);


        pt->points[0] = p0;
        pt->points[1] = p1;
        pt->points[2] = p2;
        pt++;
    }

    delete p2tCdt;
}

void Polygon::deleteTriangles()
{
    if (m_triangles)
        delete [] m_triangles;
    m_triangles = 0;
    m_triangleNum = 0;
}

void Polygon::updateBBox()
{
    m_outline->updateBBox ();
}

void Polygon::updateIndexator(int gridSize)
{
    if (m_indexator)
        delete m_indexator;
    m_indexator = new GridIndexator(this, gridSize);
}

void Polygon::updateBorderGeometry()
{
    m_outline->updateBorderGeometry();
    for(int i = 0; i < m_holes.size(); i++)
        m_holes[i]->updateBorderGeometry();
}

BBox *Polygon::bbox()
{
    return m_outline->m_bbox;
}

bool Polygon::isValid() const
{
    return m_outline->isValid();
}

void Polygon::setFlags(bool *flags)
{
    m_genMesh = flags[0];
    m_genBorders = flags[1];
    m_genCollider = flags[2];
    m_clippingSubj = flags[3];
    m_clippingClip = flags[4];
}

void Polygon::getFlags(bool *flags)
{
    flags[0] = m_genMesh;
    flags[1] = m_genBorders;
    flags[2] = m_genCollider;
    flags[3] = m_clippingSubj;
    flags[4] = m_clippingClip;
}

void Polygon::insertNext(Polygon *p)
{
        p->m_prev = this;
        p->m_next = m_next;
        if (m_next != 0)
            m_next->m_prev = p;
        m_next = p;
}

void Polygon::insertPrev(Polygon *p)
{
    p->m_next = this;
    p->m_prev = m_prev;
    if (m_prev != 0)
        m_prev->m_next = p;
    m_prev = p;
}

void Polygon::exclude(Polygon *p)
{
    Polygon *pp = p->m_prev;
    Polygon *pn = p->m_next;

    if (pp!= 0)
        pp->m_next = pn;

    if (pn!= 0)
        pn->m_prev = pp;

    p->m_prev = 0;
    p->m_next = 0;
}

bool Polygon::clipBy(t2d2::SimpPolyData *spd, std::vector<Polygon *> &outPolyVec)
{
    BBox *b = bbox();
    if (!b->isOk())
        updateBBox();

    BBox *b2 = spd->m_bbox;

//    Log()<<__FUNCTION__<<"spd bbox: X[ "<<b2->xmin<<":"<<b2->xmax<<"] Y ["<<b2->ymin<<":"<<b2->ymax<<"]";
//    Log()<<__FUNCTION__<<"poly bbox: X[ "<<b->xmin<<":"<<b->xmax<<"] Y[ "<<b->ymin<<":"<<b->ymax<<"] ";

    bool res = BBox::overlap(b, b2);

    if (!res)
        return false;

    static ClipperLib::ClipType ct = ClipperLib::ctDifference;

    ClipperLib::Clipper clipper;

    addSubjToClipper(clipper, this);
    addClipToClipper(clipper, spd);

    ClipperLib::PolyTree ptree;

    res = clipper.Execute(ct, ptree);

    if (!res)
        return false;

    buildPolyVecFromClipperTree(ptree, this, outPolyVec);

    return res;
}

void Polygon::addSubjToClipper(ClipperLib::Clipper &clipper, Polygon *poly)
{
    ClipperLib::Path clOutine;
    poly->m_outline->makeClipperLibPath(clOutine);
    clipper.AddPath(clOutine, ClipperLib::ptSubject, true);

    size_t hc = poly->m_holes.size();
    for(size_t i = 0; i < hc; i++) {
        Contour *hole = poly->m_holes[i];
        ClipperLib::Path clHole;
        hole->makeClipperLibPath(clHole);
        clipper.AddPath(clHole, ClipperLib::ptSubject, true);
    }
}

void Polygon::addClipToClipper(ClipperLib::Clipper &clipper, t2d2::SimpPolyData *spd)
{
//    ClipperLib::Path clOutline;
//    Contour::makeClipperLibPath(clOutline, spd->m_outline);
//    clipper.AddPath(clOutline, ClipperLib::ptClip, true);

//    for(int i = 0; i < spd->m_hc; i++) {
//        ClipperLib::Path clHole;
//        Contour::makeClipperLibPath(clHole, spd->m_holes[i]);
//        clipper.AddPath(clHole, ClipperLib::ptClip, true);
//    }


    clipper.AddPath(spd->m_outline.m_clPath, ClipperLib::ptClip, true);
    for(int i = 0; i < spd->m_hc; i++) {
        clipper.AddPath(spd->m_holes[i].m_clPath, ClipperLib::ptClip, true);
    }

}

void Polygon::buildPolyVecFromClipperTree(ClipperLib::PolyTree &tree, Polygon *basePoly, std::vector<Polygon *> &outVec)
{

    if (basePoly->indexator() == 0)
        basePoly->updateIndexator(10);

    ClipperLib::PolyNode *pnd = tree.GetFirst();

    while (pnd != 0) {
        if (pnd->IsHole()) {
            pnd = pnd->GetNext();
            continue;
        }

        t2d2::Polygon *poly = new Polygon(0);

        copyPolyAttributes(poly, basePoly);

        poly->m_outline->setClipperLibPath(pnd->Contour);

        int lastPrevGenIdx = Contour::restorePointAttributes(poly->m_outline, basePoly->indexator());

//        Contour::rebuildPointAttributes(poly->m_outline, lastPrevGenIdx);

        ClipperLib::PolyNodes &children = pnd->Childs;

        size_t cc = children.size();

        for(size_t i = 0; i < cc; i++) {
            ClipperLib::PolyNode *cpnd = children[i];
            if (!cpnd->IsHole())
                continue;
            Contour *hole = poly->addHole();
            hole->setClipperLibPath(cpnd->Contour);
            lastPrevGenIdx = Contour::restorePointAttributes(hole, basePoly->indexator());
//            Contour::rebuildPointAttributes(hole, lastPrevGenIdx);
        }

        outVec.push_back(poly);

        pnd = pnd->GetNext();
    }
}

void Polygon::copyPolyAttributes(Polygon *dp, Polygon *sp)
{
    dp->m_genMesh = sp->m_genMesh;
    dp->m_genBorders = sp->m_genBorders;
    dp->m_genCollider = sp->m_genCollider;
    dp->m_clippingClip = sp->m_clippingClip;
    dp->m_clippingSubj = sp->m_clippingSubj;
    dp->m_pivotX = sp->m_pivotX;
    dp->m_pivotY = sp->m_pivotY;
    dp->m_zValue = sp->m_zValue;
    dp->m_subMeshIndex = sp->m_subMeshIndex;
    UvProjection::copy(dp->m_uvProjection, sp->m_uvProjection);
}
