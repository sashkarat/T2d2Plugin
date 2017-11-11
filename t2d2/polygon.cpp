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

bool Polygon::generateBorders(std::vector<Polygon *> &outBordersPoly)
{
    size_t hs = m_holes.size();

    Borders *brdrs =  m_polyGroup->borders();

    bool accRes = false;

    for(unsigned int i = 0; i < 32; i++) {
        if (!brdrs->borderEnabled(i))
            continue;

        Border *b = brdrs->border(i);
        unsigned int mask = brdrs->mask(i);

        accRes = generateBordersAtContour(b, mask, m_outline, 1.0f, outBordersPoly) || accRes;

        for(size_t hi = 0; hi < hs; hi++)
            accRes = generateBordersAtContour(b, mask, m_holes[hi], -1.0f, outBordersPoly) || accRes;
    }

    return accRes;
}

void Polygon::complicate(int level)
{
    m_outline->complicate(level);
    size_t hs = m_holes.size();
    for(size_t i = 0; i < hs; i++)
        m_holes[i]->complicate(level);
    if (m_indexator != nullptr)
        updateIndexator(10);
}

Polygon::Polygon(PolygonGroup *pg) : m_prev(0), m_next(0)
{
    m_indexator = 0;

    m_triangles = 0;
    m_triangleNum = 0;

    m_polyGroup = pg;
    m_outline = new Contour(this, true);

    m_uvProjection = nullptr;

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

    m_skipValidation = false;
}

Polygon::~Polygon()
{

    if (m_indexator)
        delete m_indexator;

    if (m_uvProjection)
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

Triangle *Polygon::tri(int index)
{
    return &m_triangles[index];
}

bool Polygon::validate(bool withHoles)
{
    if (m_skipValidation) {
        markValid();
        return true;
    }

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

void Polygon::triangulate(bool force, bool ignoreValidation)
{
    if (m_triangles && !force)
        return;

    deleteTriangles();

    if (!ignoreValidation) {
        if (!m_outline->isValid()) {
            Log(ltError)<<__FUNCTION__<<"outline is not valid";
            return;
        }
    }

    m_outline->clearTriDataRef();

    p2t::CDT *p2tCdt = new p2t::CDT(m_outline->m_data);

    size_t hs = m_holes.size();

    for(size_t i = 0; i < hs; i++) {
        Contour *holePtr = m_holes[i];

        if (!ignoreValidation) {
            if (!holePtr->isValid())
                continue;
        }

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

        pt->m_p[0] = p0;
        pt->m_p[1] = p1;
        pt->m_p[2] = p2;
        st->_extData = reinterpret_cast<void*>(pt);
        pt++;
    }

    for(unsigned int i = 0; i < m_triangleNum; i++) {
        p2t::Triangle *st = tri[i];

        p2t::Triangle *n0 = st->GetNeighbor(0);
        p2t::Triangle *n1 = st->GetNeighbor(1);
        p2t::Triangle *n2 = st->GetNeighbor(2);

        t2d2::Triangle *t = reinterpret_cast<t2d2::Triangle*>(st->_extData);
        t->m_neighbours[0] = (n0 != nullptr)?reinterpret_cast<t2d2::Triangle*>(n0->_extData):nullptr;
        t->m_neighbours[1] = (n1 != nullptr)?reinterpret_cast<t2d2::Triangle*>(n1->_extData):nullptr;
        t->m_neighbours[2] = (n2 != nullptr)?reinterpret_cast<t2d2::Triangle*>(n2->_extData):nullptr;
    }

    delete p2tCdt;
}

void Polygon::deleteTriangles()
{
    if (m_triangles)
        delete [] m_triangles;
    m_triangles = nullptr;
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

BBox *Polygon::bbox()
{
    return m_outline->m_bbox;
}

void Polygon::createUvProjection(float *matrix)
{
    if (m_uvProjection)
        delete m_uvProjection;
    m_uvProjection = new UvProjection();

    if (matrix)
        m_uvProjection->setMatrix4x4(matrix);
}

void Polygon::projectUV()
{
    if (m_uvProjection == nullptr)
        return;

    m_outline->projectUV(m_uvProjection);

    size_t hc = m_holes.size();
    for(size_t hi =0; hi < hc; hi++)
        m_holes[hi]->projectUV(m_uvProjection);
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

    clipper.StrictlySimple(true);

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
    clipper.AddPath(spd->m_outline.m_clPath, ClipperLib::ptClip, true);
    for(int i = 0; i < spd->m_hc; i++) {
        clipper.AddPath(spd->m_holes[i].m_clPath, ClipperLib::ptClip, true);
    }
}

void Polygon::buildPolyVecFromClipperTree(ClipperLib::PolyTree &tree, Polygon *basePoly, std::vector<Polygon *> &outVec)
{
    if (basePoly->m_indexator == nullptr)
        basePoly->updateIndexator(10);

    basePoly->triangulate(false, true);

    ClipperLib::PolyNode *pnd = tree.GetFirst();

    while (pnd != 0) {
        if (pnd->IsHole() || (pnd->Contour.size() < 3)) {
            pnd = pnd->GetNext();
            continue;
        }

        t2d2::Polygon *poly = new Polygon(0);

        copyPolyAttributes(poly, basePoly);

        poly->m_outline->setClipperLibPath(pnd->Contour);

        Contour::restorePointAttributes(poly->m_outline, basePoly);

        ClipperLib::PolyNodes &children = pnd->Childs;

        size_t cc = children.size();

        for(size_t i = 0; i < cc; i++) {
            ClipperLib::PolyNode *cpnd = children[i];
            if (!cpnd->IsHole())
                continue;
            if (cpnd->Contour.size() == 0)
                continue;
            Contour *hole = poly->addHole();
            hole->setClipperLibPath(cpnd->Contour);
            Contour::restorePointAttributes(hole, basePoly);
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
    if (sp->m_uvProjection) {
        dp->createUvProjection(nullptr);
        UvProjection::copy(dp->m_uvProjection, sp->m_uvProjection);
    }
}

bool Polygon::generateBordersAtContour(Border *b, unsigned int bmask, Contour *cntr, float holeFactor, std::vector<Polygon *> &outBordersPoly)
{
    size_t lastI = outBordersPoly.size();

    std::vector<ClipperLib::Path *> segments;

    unsigned int cLen = cntr->length();

//    Log()<<__FUNCTION__<<" start. bmask: "<<bmask<<" cLen: "<<cLen<<" offsetScale:"<<holeFactor;

    if (cLen < 2)
        return false;

    t2d2::Point *p = cntr->getPoint(0);
    t2d2::Point *sp = p;

    ClipperLib::Path *segment = nullptr;

//    Log()<<__FUNCTION__<<" enter into a segments generation loop";

//    int pidx = 0;

    do {
//        Log()<<__FUNCTION__<<"p: "<<p->x<<p->y<<"mask: "<<(p->m_borderFlags & bmask)<<"idx:"<<pidx++;
        if (p->m_borderFlags & bmask) {
            if (segment == nullptr)
                segment = new ClipperLib::Path();

            segment->push_back( ClipperLib::IntPoint(
                                   p->x * FLOAT2CLINT,
                                   p->y * FLOAT2CLINT ));
//            Log()<<__FUNCTION__<<"push point"<<(*segment)[segment->size()-1];
        } else if (segment != nullptr) {
            segments.push_back(segment);
//            Log()<<__FUNCTION__<<"push segment";
            segment = nullptr;
        }

        p = p->m_np;

    } while (p != sp);

    if (segment != nullptr)
        segments.push_back(segment);


//    Log()<<__FUNCTION__<<"segments number: "<<segments.size();
//    for(size_t i = 0; i < segments.size(); i++) {
//        Log()<<__FUNCTION__<<"i"<<i<<"size"<<segments[i]->size();
//    }

    if (segments.size() == 0)
        return false;


    UvPathProjecton  uvp;
    float midDelta = (b->offset() + b->width() / 2) * holeFactor * FLOAT2CLINT;

    float oDelta = b->offset() * holeFactor * FLOAT2CLINT;
    float wDelta = (b->offset() + b->width()) * holeFactor * FLOAT2CLINT;

    float halfDelta = (wDelta - oDelta) / 2;

    generateUvPathProjData(uvp, cntr, midDelta);

    ClipperLib::Path zpClPath;

    uvp.createZeroPointMiterClip(zpClPath, b->width());

//    Log::logClipperPath(__FUNCTION__, "zpClPath", zpClPath);

//    Polygon *zpPoly = new Polygon(m_polyGroup);
//    zpPoly->m_outline->setClipperLibPath(zpClPath);
//    outBordersPoly.push_back(zpPoly);


//    ClipperLib::Path &midLinePath = uvp.baseLineClPath();

    ClipperLib::Path zpCp;
    zpCp.resize(4);

    uvp.computeZControlPoints(b->width(), 1.0f, zpCp[0], zpCp[1], zpCp[2], zpCp[3]);

    ClipperLib::IntPoint zp = uvp.baseLineClPath()[0];

//    Log()<<__FUNCTION__<<"seg 0 size: "<<segments[0]->size();

    if (segments.size() == 1 && segments[0]->size() == cLen) {
//        Log()<<__FUNCTION__<<"full border";

        BorderClipperData bcd(b);

//        generateFullBorder( *(segments[0]), wDelta, oDelta, zpCp, zpClPath, bcd);

        generateFullBorder( *(segments[0]), wDelta, oDelta, zp, bcd);

        buildPolyFromBCD(bcd, uvp, (holeFactor < 0.0f), outBordersPoly);

    } else {
        p = sp->m_pp;

        bool sliceLast = false;

        if ((p->m_borderFlags & sp->m_borderFlags) & bmask) {

//            Log()<<__FUNCTION__<<"merge first and last";

            ClipperLib::Path *fs = segments[0];
            ClipperLib::Path *ls = segments[segments.size() - 1];

            ls->insert(ls->end(), fs->begin(), fs->end());

            segments.erase(segments.begin());

            delete fs;

            sliceLast = true;
        }

        size_t lsi = segments.size() - 1;

        for(size_t i = 0; i < segments.size(); i++) {
            BorderClipperData bcd(b);

            int erCode = generateBorderSegment(*(segments[i]), midDelta, halfDelta, bcd);


            if (erCode == 0) {
                if ((i == lsi) && sliceLast) {
                    BorderClipperData bcdA(b), bcdB(b);
                    sliceBcd(bcd, zpCp, zpClPath, bcdA, bcdB);
                    buildPolyFromBCD(bcdA, uvp, (holeFactor < 0.0f), outBordersPoly);
                    buildPolyFromBCD(bcdB, uvp, (holeFactor < 0.0f), outBordersPoly);
                } else {
                    buildPolyFromBCD(bcd, uvp, (holeFactor < 0.0f), outBordersPoly);
                }
            } else if (erCode > 0) {
                generateFullBorder( *(segments[0]), wDelta, oDelta, zp, bcd);
//                generateFullBorder( *(segments[0]), wDelta, oDelta, zpCp, zpClPath, bcd);
                buildPolyFromBCD(bcd, uvp, (holeFactor < 0.0f), outBordersPoly);
                break;
            } else {
                Log(ltError)<<__FUNCTION__<<"border segment gen. failed";
            }
        }
    }

    for(size_t i = 0; i < segments.size(); i++)
        delete segments[i];


    for(size_t pi = lastI; pi < outBordersPoly.size(); pi++) {
        Polygon *bp = outBordersPoly[pi];

        copyPolyAttributes(bp, this);

        bp->m_genMesh = true;
        bp->m_genCollider = false;
        bp->m_subMeshIndex = b->subMeshIndex();
        bp->m_zValue += b->zOffset();
        bp->m_clippingClip = false;
        bp->m_skipValidation = true;
    }
    return true;
}

void Polygon::generateZeroClipPath(ClipperLib::IntPoint &zO, ClipperLib::IntPoint &zW, bool holeFactor, ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zClPath)
{

    float dx = zW.X - zO.X;
    float dy = zW.Y - zO.Y;

    t2d2::util::fastnorm(dx, dy);

    dx *= 10;
    dy *= 10;

    ClipperLib::IntPoint _zO = zO;
    ClipperLib::IntPoint _zW = zW;

    _zO.X -= dx;
    _zO.Y -= dy;

    _zW.X += dx;
    _zW.Y += dy;

    zpCtrlPoints.resize(4);
    if (holeFactor) {
        t2d2::util::cl::normalOffsetP0(_zO, _zW, 10, zpCtrlPoints[0]);
        t2d2::util::cl::normalOffsetP0(_zO, _zW, -10, zpCtrlPoints[1]);
        t2d2::util::cl::normalOffsetP1(_zO, _zW, 10, zpCtrlPoints[2]);
        t2d2::util::cl::normalOffsetP1(_zO, _zW, -10, zpCtrlPoints[3]);
    } else {
        t2d2::util::cl::normalOffsetP0(_zW, _zO, 10, zpCtrlPoints[0]);
        t2d2::util::cl::normalOffsetP0(_zW, _zO, -10, zpCtrlPoints[1]);
        t2d2::util::cl::normalOffsetP1(_zW, _zO, 10, zpCtrlPoints[2]);
        t2d2::util::cl::normalOffsetP1(_zW, _zO, -10, zpCtrlPoints[3]);
    }

    zClPath.resize(4);
    zClPath[0] = zpCtrlPoints[0];
    zClPath[1] = zpCtrlPoints[2];
    zClPath[2] = zpCtrlPoints[3];
    zClPath[3] = zpCtrlPoints[1];
}

ClipperLib::IntPoint Polygon::localizeZeroPoint(ClipperLib::Path &path, int zIndex, ClipperLib::IntPoint &zp2)
{
    int ps = static_cast<int>(path.size());
    ClipperLib::IntPoint &p0 = path[zIndex];

    ClipperLib::IntPoint &pP = path[ t2d2::util::_index(zIndex-1, ps)];
    ClipperLib::IntPoint &pN = path[ t2d2::util::_index(zIndex+1, ps)];

//    Log()<<__FUNCTION__<<"pP-p0-pN"<<pP<<p0<<pN;

    float cosAP = t2d2::util::fastabs(t2d2::util::cl::cosA(zp2, p0, pP, p0));
    float cosAN = t2d2::util::fastabs(t2d2::util::cl::cosA(zp2, p0, p0, pN));

//    Log()<<__FUNCTION__<<"cosAP"<<cosAP<<"cosAN"<<cosAN;

    if (cosAP > 0.85f || cosAN > 0.85f)
        return (cosAP > cosAN) ? offsetZeroPoint(p0, pN, cosAN) : offsetZeroPoint(p0, pP, cosAP);

    return path[zIndex];
}

ClipperLib::IntPoint Polygon::offsetZeroPoint(ClipperLib::IntPoint &p0, ClipperLib::IntPoint &p1, float cosA)
{
    float dx = p1.X - p0.X;
    float dy = p1.Y - p1.Y;
    t2d2::util::fastnorm(dx, dy);
    ClipperLib::IntPoint zp;
    zp.X = p0.X + dx * (10 / (1-cosA));
    zp.Y = p0.Y + dy * (10 / (1-cosA));
    return zp;
}

void Polygon::generateFullBorder(ClipperLib::Path &path, float wDelta, float oDelta, ClipperLib::IntPoint &zeroPoint, Polygon::BorderClipperData &bcd)
{
//    Log()<<__FUNCTION__<<"zeroPoint:"<<zeroPoint;

    ClipperLib::ClipperOffset co;
    co.AddPath(path, ClipperLib::jtMiter, ClipperLib::etClosedPolygon);

    ClipperLib::Paths oSol, wSol;

    co.Execute(oSol, oDelta);
    co.Execute(wSol, wDelta);

    if (wSol.size() == 0 || oSol.size() == 0)
        return;

    ClipperLib::Path &wSolP = wSol[0];
    ClipperLib::Path &oSolP = oSol[0];

    t2d2::util::cl::savePath(wSolP, "w://tmp//_0_wsolP.path");
    t2d2::util::cl::savePath(oSolP, "w://tmp//_0_osolP.path");

    int zidxW = t2d2::util::cl::findNearest(wSolP, zeroPoint, 1e30);
    if (zidxW < 0) {
        Log(ltError)<<__FUNCTION__<<"unable to find zeroPoint at wSolP. zPoint:"<<zeroPoint;
        return;
    }

    int zidxO = t2d2::util::cl::findNearest(oSolP, zeroPoint, 1e30);
    if (zidxO < 0) {
        Log(ltError)<<__FUNCTION__<<"unable to find zeroPoint at oSolP. zPoint:"<<zeroPoint;
        return;
    }

    ClipperLib::IntPoint zW = localizeZeroPoint(wSolP, zidxW, oSolP[zidxO]);

    ClipperLib::IntPoint zO = localizeZeroPoint(oSolP, zidxO, zW);

//    Log()<<__FUNCTION__<<"zO"<<zO<<"zW"<<zW;

    ClipperLib::Path zpCtrlPoints, zpClPath;

    generateZeroClipPath(zO, zW, (wDelta < oDelta),  zpCtrlPoints, zpClPath);

    t2d2::util::cl::savePath(zpClPath, "w://tmp//_0_zpClPath.path");

    ClipperLib::Clipper cl;
    cl.StrictlySimple(true);

    ClipperLib::Paths sol;
    cl.AddPath(wSolP, ClipperLib::ptSubject, true);
    cl.AddPath(oSolP, ClipperLib::ptSubject, true);
    cl.AddPath(zpClPath, ClipperLib::ptClip, true);

    cl.Execute(ClipperLib::ctDifference, sol);

    if (sol.size() == 0)
        return;

    ClipperLib::Path &contour = sol[0];

    std::vector<int> indices;

    t2d2::util::cl::findNearest(contour, zpCtrlPoints, 100, indices);

    bcd.m_contour.clear();
    bcd.m_contour.insert(bcd.m_contour.begin(), contour.begin(), contour.end());
    bcd.oS() = indices[0];
    bcd.oE() = indices[1];
    bcd.wS() = indices[2];
    bcd.wE() = indices[3];

}

void Polygon::generateFullBorder(ClipperLib::Path &path, float wDelta, float oDelta,
                                 ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zpClPath,
                                 Polygon::BorderClipperData &bcd)
{
    ClipperLib::ClipperOffset co;
    co.AddPath(path, ClipperLib::jtMiter, ClipperLib::etClosedPolygon);

    ClipperLib::Paths oSol, wSol;

    co.Execute(oSol, oDelta);
    co.Execute(wSol, wDelta);

    if (wSol.size() == 0 || oSol.size() == 0)
        return;

    ClipperLib::Path &wSolP = wSol[0];
    ClipperLib::Path &oSolP = oSol[0];

    ClipperLib::Clipper cl;
    ClipperLib::Paths sol;
    cl.AddPath(wSolP, ClipperLib::ptSubject, true);
    cl.AddPath(oSolP, ClipperLib::ptSubject, true);
    cl.AddPath(zpClPath, ClipperLib::ptClip, true);

    t2d2::util::cl::savePath(wSolP, "w://tmp//__wsolP.path");
    t2d2::util::cl::savePath(oSolP, "w://tmp//__osolP.path");
    t2d2::util::cl::savePath(zpClPath, "w://tmp//__zpClPath.path");

    cl.Execute(ClipperLib::ctDifference, sol);

    if (sol.size() == 0)
        return;

    ClipperLib::Path &contour = sol[0];

    std::vector<int> indices;

    t2d2::util::cl::findNearest(contour, zpCtrlPoints, 100, indices);


//    Log::logClipperPath(__FUNCTION__, "contour", contour);

//    Log()<<__FUNCTION__<<"idx oS"<<indices[0];
//    Log()<<__FUNCTION__<<"idx oE"<<indices[1];
//    Log()<<__FUNCTION__<<"idx wS"<<indices[2];
//    Log()<<__FUNCTION__<<"idx wE"<<indices[3];

    bcd.m_contour.clear();
    bcd.m_contour.insert(bcd.m_contour.begin(), contour.begin(), contour.end());
    bcd.oS() = indices[0];
    bcd.oE() = indices[1];
    bcd.wS() = indices[2];
    bcd.wE() = indices[3];
}

int Polygon::generateBorderSegment(ClipperLib::Path &p, float midDelta, float halfDelta, Polygon::BorderClipperData &bcd)
{

//    Log::logClipperPath(__FUNCTION__, "segment", p);

//    t2d2::util::cl::savePath(p, "w://tmp//_1_segment.path");

    ClipperLib::Path midLineCp;
    midLineCp.resize(2);

    t2d2::util::cl::offsetPoint(p, 0, true, midDelta, midLineCp[0]);
    t2d2::util::cl::offsetPoint(p, p.size()-1, true, midDelta, midLineCp[1]);

//    t2d2::util::cl::savePath(midLineCp, "w://tmp//_1_mlCP.path");

    ClipperLib::ClipperOffset co;
    co.AddPath(p, ClipperLib::jtMiter, ClipperLib::etOpenButt);
    ClipperLib::Paths preSol;

    co.Execute(preSol, t2d2::util::fastabs(midDelta));

    if (preSol.size() == 0)
        return -1;

     ClipperLib::Path &psol = preSol[0];

//     t2d2::util::cl::savePath(psol, "w://tmp//_1_psolMl.path");


    std::vector<int> midIdc;

    t2d2::util::cl::findNearest(psol, midLineCp, 2000, midIdc);

//    Log()<<__FUNCTION__<<"midIdc"<<midIdc[0]<<midIdc[1];

    if (midIdc[0] < 0 || midIdc[1] < 0)
        return -1;

    if (midIdc[0] == midIdc[1] )
        return 1;

    int pss = psol.size();

    ClipperLib::Path midLine;

    int pi = midIdc[0];
    int idx;

    int inc = (halfDelta > 0) ? 1 : -1;

    do {
        idx = t2d2::util::_index(pi, pss);
//        Log()<<__FUNCTION__<<"pi"<<pi<<"idx"<<idx;
        midLine.push_back(psol[idx]);
        pi+= inc;

    } while (idx != midIdc[1]);


    ClipperLib::CleanPolygon(midLine);

//    Log::logClipperPath(__FUNCTION__, "midLine", midLine);

//    t2d2::util::cl::savePath(midLine, "w://tmp//_1_midLine.path");

    co.Clear();
    co.AddPath(midLine, ClipperLib::jtMiter, ClipperLib::etOpenButt);

//    ClipperLib::Paths sol;

    ClipperLib::PolyTree sol;

    co.Execute(sol, t2d2::util::fastabs(halfDelta));


    ClipperLib::PolyNode *node = sol.GetFirst();
    ClipperLib::PolyNode *fn = nullptr;
    int solSize = 0;
    while (node != nullptr) {
        if (!node->IsHole()) {
            if (fn== nullptr)
                fn = node;

//            t2d2::util::cl::savePath(node->Contour, std::string("w://tmp//_1_") + std::to_string(solSize) + "_sol.path");
            solSize++;
        }

        node = node->GetNext();
    }

    if (solSize == 0)
        return -1;

    if (solSize > 1)
        return 1;

    ClipperLib::Path &bp = fn->Contour;

    size_t mls = midLine.size();
    size_t bps = bp.size();

    float bcoeff = static_cast<float>(bps) / static_cast<float>(mls);

//    Log()<<__FUNCTION__<<"mls"<<mls<<"bps"<<bps<<"bcoeff:"<<bcoeff;

    if (bcoeff < 1.1f)
        return 1;


//    Log::logClipperPath(__FUNCTION__, "brdr", bp);

    // order: oS, oE, wS, wE

    ClipperLib::Path cp;
    cp.resize(4);

    int lpIdx = midLine.size()-1;

    halfDelta = t2d2::util::fastabs(halfDelta);

    t2d2::util::cl::offsetPoint(midLine, 0, true, -halfDelta, cp[0]);
    t2d2::util::cl::offsetPoint(midLine, lpIdx, true, -halfDelta, cp[1]);
    t2d2::util::cl::offsetPoint(midLine, 0, true, halfDelta, cp[2]);
    t2d2::util::cl::offsetPoint(midLine, lpIdx, true, halfDelta, cp[3]);


//    Log()<<__FUNCTION__<<"oS"<<cp[0];
//    Log()<<__FUNCTION__<<"oE"<<cp[1];
//    Log()<<__FUNCTION__<<"wS"<<cp[2];
//    Log()<<__FUNCTION__<<"wE"<<cp[3];


    ClipperLib::Path ctrlPath;
    ctrlPath.push_back(cp[0]);
    ctrlPath.push_back(cp[1]);
    ctrlPath.push_back(cp[2]);
    ctrlPath.push_back(cp[3]);


//    t2d2::util::cl::savePath(ctrlPath, "w://tmp//_1_ctrl.path");

    std::vector<int> oi;

    util::cl::findNearest(bp, cp, 2000, oi);

//    Log()<<__FUNCTION__<<"idx: oS"<<oi[0];
//    Log()<<__FUNCTION__<<"idx: oE"<<oi[1];
//    Log()<<__FUNCTION__<<"idx: wS"<<oi[2];
//    Log()<<__FUNCTION__<<"idx: wE"<<oi[3];


    if (oi[0] >= 0 && oi[1] >= 0 && oi[2] >= 0 && oi[3] >= 0) {

        bcd.m_indices[0] = oi[0];
        bcd.m_indices[1] = oi[1];
        bcd.m_indices[2] = oi[2];
        bcd.m_indices[3] = oi[3];
        bcd.m_contour.insert(bcd.m_contour.begin(), bp.begin(), bp.end());
//        t2d2::util::cl::savePath(bcd.m_contour, "w://tmp//_1_contour.path");
        return 0;
    }
    bcd.m_contour.insert(bcd.m_contour.begin(), bp.begin(), bp.end());


//    t2d2::util::cl::savePath(bcd.m_contour, "w://tmp//_11contour.path");
    return 1;
}

//bool Polygon::generateBorderSegment(ClipperLib::Path &p, ClipperLib::Path &fullMidLinePath, float midDelta, float halfDelta, Polygon::BorderClipperData &bcd)
//{
//    ClipperLib::ClipperOffset co;
//    co.AddPath(p, ClipperLib::jtMiter, ClipperLib::etOpenButt);
//    ClipperLib::Paths preSol;

//    co.Execute(preSol, t2d2::util::fastabs(midDelta));

//    if (preSol.size() == 0)
//        return false;

//    ClipperLib::Path &psol = preSol[0];


//    ClipperLib::IntPoint midLineStartPoint;

//    t2d2::util::cl::offsetPoint(p, 0, true, midDelta, midLineStartPoint);


//    int startIndex = t2d2::util::cl::findNearest(fullMidLinePath, midLineStartPoint, 10000);

//    if (startIndex < 0) {
//        Log(ltError)<<__FUNCTION__<<"unable to find midLineStartPoint";
//        return false;
//    }

//    ClipperLib::Path midLine;


//    size_t fmlS = fullMidLinePath.size();
//    for(size_t i = 0; i <fmlS; i++) {

//        size_t idx = (i + startIndex) % fmlS;

//        ClipperLib::IntPoint &pnt = fullMidLinePath[idx];

//        int index =  t2d2::util::cl::findNearest(psol, pnt, 1);
//        if (index < 0)
//            continue;

//        midLine.push_back(pnt);
//    }


//    t2d2::util::cl::savePath(midLine, "w://tmp//__midLine.path");


//    co.Clear();
//    co.AddPath(midLine, ClipperLib::jtMiter, ClipperLib::etOpenButt);

//    ClipperLib::Paths sol;

//    co.Execute(sol, t2d2::util::fastabs(halfDelta));

//    if (sol.size() == 0) {
//        Log(ltError)<<__FUNCTION__<<"unable to generate border path";
//        return false;
//    }


//    for(size_t i = 0; i < sol.size(); i++) {
//        t2d2::util::cl::savePath(sol[i], std::string("w://tmp//__brdrSol_")  + std::to_string(i) + ".path");
//    }


//    return false;

//}

void Polygon::sliceBcd(Polygon::BorderClipperData &bcd, ClipperLib::Path &zpCtrlPoints, ClipperLib::Path &zpClPath, Polygon::BorderClipperData &bcdA, Polygon::BorderClipperData &bcdB)
{

//    Log::logClipperPath(__FUNCTION__, "source bcd", bcd.m_contour);


    ClipperLib::Path cp;
    cp.resize(4);

    cp[0] = bcd.m_contour[bcd.oS()];
    cp[1] = bcd.m_contour[bcd.oE()];
    cp[2] = bcd.m_contour[bcd.wS()];
    cp[3] = bcd.m_contour[bcd.wE()];

//    Log()<<__FUNCTION__<<"src oS"<<cp[0];
//    Log()<<__FUNCTION__<<"src oE"<<cp[1];
//    Log()<<__FUNCTION__<<"src wS"<<cp[2];
//    Log()<<__FUNCTION__<<"src wE"<<cp[3];



//    Log()<<__FUNCTION__<<"zpCtrlPoints oS"<<zpCtrlPoints[0];
//    Log()<<__FUNCTION__<<"zpCtrlPoints oE"<<zpCtrlPoints[1];
//    Log()<<__FUNCTION__<<"zpCtrlPoints wS"<<zpCtrlPoints[2];
//    Log()<<__FUNCTION__<<"zpCtrlPoints wE"<<zpCtrlPoints[3];


    ClipperLib::Clipper cl;

    cl.AddPath(bcd.m_contour, ClipperLib::ptSubject, true);
    cl.AddPath(zpClPath, ClipperLib::ptClip, true);

    ClipperLib::PolyTree sol;

    cl.Execute(ClipperLib::ctDifference, sol);

    ClipperLib::PolyNode *pnd = sol.GetFirst();


    int a = 0;

    while (pnd!= nullptr) {

        if (!pnd->IsHole()) {
//            Log()<<__FUNCTION__<<" ";
//            Log::logClipperPath(__FUNCTION__, "solution", pnd->Contour);

            std::vector<int> cpi;
            t2d2::util::cl::findNearest(pnd->Contour, cp, 1, cpi);

//            Log()<<__FUNCTION__<<" cp idx oS "<<cpi[0];
//            Log()<<__FUNCTION__<<" cp idx oE "<<cpi[1];
//            Log()<<__FUNCTION__<<" cp idx wS "<<cpi[2];
//            Log()<<__FUNCTION__<<" cp idx wE "<<cpi[3];

            if (cpi[0] < 0)
                cpi[0] = t2d2::util::cl::findNearest(pnd->Contour, zpCtrlPoints[0], 10);
            if (cpi[1] < 0)
                cpi[1] = t2d2::util::cl::findNearest(pnd->Contour, zpCtrlPoints[1], 10);
            if (cpi[2] < 0)
                cpi[2] = t2d2::util::cl::findNearest(pnd->Contour, zpCtrlPoints[2], 10);
            if (cpi[3] < 0)
                cpi[3] = t2d2::util::cl::findNearest(pnd->Contour, zpCtrlPoints[3], 10);

//            Log()<<__FUNCTION__<<"res cp idx oS "<<cpi[0];
//            Log()<<__FUNCTION__<<"res cp idx oE "<<cpi[1];
//            Log()<<__FUNCTION__<<"res cp idx wS "<<cpi[2];
//            Log()<<__FUNCTION__<<"res cp idx wE "<<cpi[3];

            BorderClipperData &outBcd = (a==0) ? bcdA : bcdB;

            outBcd.m_contour.insert(outBcd.m_contour.begin(), pnd->Contour.begin(), pnd->Contour.end());
            outBcd.m_indices[0] = cpi[0];
            outBcd.m_indices[1] = cpi[1];
            outBcd.m_indices[2] = cpi[2];
            outBcd.m_indices[3] = cpi[3];

            if (++a == 2)
                break;
        }

        pnd = pnd->GetNext();
    }
}

void Polygon::cleanupBCD(Polygon::BorderClipperData &bcd)
{
    ClipperLib::Path cp;
    cp.resize(4);

    if (bcd.oS() >= 0)
        cp[0] = bcd.m_contour[bcd.oS()];

    if (bcd.oE() >= 0)
        cp[1] = bcd.m_contour[bcd.oE()];

    if (bcd.wS() >= 0)
        cp[2] = bcd.m_contour[bcd.wS()];

    if (bcd.wE() >= 0)
        cp[3] = bcd.m_contour[bcd.wE()];

    ClipperLib::Path presol;

    presol.push_back(bcd.m_contour[0]);

    size_t li = 1;

    float dt2 = 10;

    size_t cs = bcd.m_contour.size();

    for(size_t  i = 1; i < cs; i++) {
        ClipperLib::IntPoint &pA = presol[li];
        ClipperLib::IntPoint &pB = bcd.m_contour[i];

        float d2 =  t2d2::util::cl::dist(pA, pB);

        if (d2 <= dt2) {
            Log()<<__FUNCTION__<<"points are too close:"<<pA<<" - "<<pB<<" i"<<i<<"d"<<d2;
        } else {
            presol.push_back(pB);
            li++;
        }
    }


    ClipperLib::Paths outPaths;

    ClipperLib::SimplifyPolygon(presol, outPaths);

    Log()<<__FUNCTION__<<"simplification done. outPaths size:"<<outPaths.size();

    if (outPaths.size() == 0)
        return;


    ClipperLib::Path &sol = outPaths[0];
    bcd.m_contour.clear();
    bcd.m_contour.insert(bcd.m_contour.begin(), sol.begin(), sol.end());

    std::vector<int> ni;

    t2d2::util::cl::findNearest(bcd.m_contour, cp, 100, ni);

    bcd.m_indices[0] = ni[0];
    bcd.m_indices[1] = ni[1];
    bcd.m_indices[2] = ni[2];
    bcd.m_indices[3] = ni[3];

    Log()<<__FUNCTION__<<"bcd contour size: "<<bcd.m_contour.size();

    if (bcd.m_contour.size() > 60) {
        Log()<<__FUNCTION__<<"57"<<bcd.m_contour[57];
        Log()<<__FUNCTION__<<"58"<<bcd.m_contour[58];
        Log()<<__FUNCTION__<<"59"<<bcd.m_contour[59];
        Log()<<__FUNCTION__<<"60"<<bcd.m_contour[60];
    }
}

void Polygon::buildPolyFromBCD(Polygon::BorderClipperData &bcd, UvPathProjecton &uvp, bool isHole, std::vector<Polygon *> &outPoly)
{
    if (bcd.m_contour.size() < 3) {
        Log(ltError)<<__FUNCTION__<<"bcd.contour size: "<<bcd.m_contour.size();
        return;
    }

//    cleanupBCD(bcd);

    Polygon *poly = new Polygon(m_polyGroup);

    poly->m_outline->setClipperLibPath(bcd.m_contour);

    generateUvPathProjection(uvp, bcd, poly->m_outline, isHole);

    outPoly.push_back(poly);
}

void Polygon::generateUvPathProjData(UvPathProjecton &uvpd, Contour *cntr, float delta)
{
    ClipperLib::Path clPath;

    cntr->makeClipperLibPath(clPath);


    ClipperLib::ClipperOffset clO;

    clO.AddPath(clPath, ClipperLib::jtMiter, ClipperLib::etClosedPolygon);

    ClipperLib::Paths solPaths;

    clO.Execute(solPaths, delta);

    uvpd.build(solPaths[0]);

}

void Polygon::generateUvPathProjection(UvPathProjecton &uvp, Polygon::BorderClipperData &bcd, Contour *cntr, bool isHole)
{
    float valA = isHole ? 1.05f : 0.0f;
    float valB = isHole ? 0.0f : 1.05f;

    size_t pc = cntr->length();

    for(size_t pi = 0; pi < pc; pi++) {

        t2d2::Point *p = cntr->getPoint(pi);

        int index = uvp.findNearest(p->x, p->y);

        if (index == 0) {
            if (pi == bcd.wE() || pi == bcd.oE())
                index = uvp.len()-1;
        }

        float _u = uvp.pos(index);
        float _v = t2d2::util::inClosedRange(pi, bcd.oE(), bcd.oS())? valA : valB;

        p->m_u = bcd.m_border->uOffset() + _u * bcd.m_border->uScale();
        p->m_v = bcd.m_border->vOffset() + _v * bcd.m_border->vScale();

    }
}

