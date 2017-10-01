#include "polygongroup.h"

using namespace t2d2;

PolygonGroup::PolygonGroup() :
    m_polygon(nullptr), m_clippingClip(nullptr)
{
    m_borders = new Borders();
    m_colliderArea = 0;
    m_colliderComX = m_colliderComY = 0;
    m_colliderPathNum = 0;
}

PolygonGroup::~PolygonGroup()
{
    clippingClearClipData();
    delete m_borders;
    deletePolygons();
}

Polygon *PolygonGroup::addPolygon()
{
    if (m_polygon == nullptr) {
        m_polygon = new Polygon(this);
        return m_polygon;
    } else {
        Polygon *np = new Polygon(this);
        m_polygon->findLast()->insertNext(np);
        return np;
    }
}

Polygon *PolygonGroup::addPolygon(Polygon *poly)
{
    if (m_polygon == nullptr) {
        m_polygon = poly;
        poly->m_prev = nullptr;
    } else {
       m_polygon->findLast()->insertNext(poly);
    }
    poly->m_polyGroup = this;
    return poly;
}

void PolygonGroup::addPolygons(std::vector<Polygon *> &polyVec)
{
    size_t c = polyVec.size();
    if (c == 0)
        return;


    Polygon* poly = addPolygon(polyVec[0]);
    poly->updateIndexator(10);

    for(size_t i = 1; i < c; i++) {
        Polygon *np = polyVec[i];
        poly->updateBBox();
        poly->updateIndexator(10);
        poly->insertNext(np);
        poly->m_polyGroup = this;
        np->m_next = nullptr;

//        BBox *b = poly->outline()->bbox();
//        Log()<<__FUNCTION__<<"poly bbox: X[ "<<b->xmin<<":"<<b->xmax<<"] Y[ "<<b->ymin<<":"<<b->ymax<<"] ";

        poly = np;
    }
}

void PolygonGroup::deletePolygon(Polygon *p)
{
    if (p == m_polygon)
        m_polygon = p->next();
    Polygon::exclude(p);
    delete p;
}

MeshCash *PolygonGroup::createMeshCash(int subMeshNum, bool validate)
{
    MeshCash *mcash = new MeshCash(this);
    mcash->build(subMeshNum, validate);
    return mcash;
}

void PolygonGroup::deleteMeshCash(MeshCash *mcash)
{
    if (mcash)
        delete mcash;
}

ColliderCash *PolygonGroup::createColliderCash()
{
    ColliderCash *ccash = new ColliderCash(this);
    ccash->build();
    return ccash;
}

void PolygonGroup::deleteColliderCash(ColliderCash * ccash)
{
    if (ccash)
        delete ccash;
}

void PolygonGroup::updateColliderGeometricValues()
{
    Polygon *poly = m_polygon;

    m_colliderComX = m_colliderComY = 0;
    m_colliderArea = 0;
    m_colliderPathNum = 0;

    while (poly != nullptr) {

        if (poly->genCollider()) {

            poly->updateCOM();

            float a = poly->getArea();
            float x = poly->getComX();
            float y = poly->getComY();


            m_colliderArea += a;

            m_colliderComX += x * a;
            m_colliderComY += y * a;

            m_colliderPathNum++;
        }

        poly = poly->next();
    }

    if (m_colliderPathNum == 0)
        return;

    if (m_colliderArea == 0) {
        m_colliderComX = m_colliderComY = 0;
        return;
    }

    m_colliderComX /= m_colliderArea;
    m_colliderComY /= m_colliderArea;
}


void PolygonGroup::saveToFile(PolygonGroup *pg, std::ofstream &fs)
{
    Polygon *poly = pg->m_polygon;

    char nf = (poly)?1:0;

    fs.write(&nf, 1);

    while (poly != nullptr ) {
        Polygon::saveToFile(poly, fs);
        poly = poly->m_next;
        nf = (poly)?1:0;
        fs.write(&nf, 1);
    }

    t2d2::Borders::saveToFile(pg->m_borders, fs);
}

PolygonGroup *PolygonGroup::loadFromFile(std::ifstream &fs)
{

    PolygonGroup *pg = new PolygonGroup();
    char nf = 1;
    while (!fs.eof()) {
        fs.read(&nf, 1);
        if (nf) {
            Polygon *poly = pg->addPolygon();
            Polygon::loadFromFile(poly, fs);
        } else {
            break;
        }
    }

    t2d2::Borders::loadFromFile(pg->m_borders, fs);

    return pg;
}

bool PolygonGroup::clipBy(PolygonGroup *clipperPg, float *trMtx)
{
    Polygon *clipperPoly = clipperPg->polygon();

    bool res = false;

    while (clipperPoly != nullptr) {

        if (clipperPoly->clippingClip()) {

            SimpPolyData *spd = Polygon::createSimpPolyData(clipperPoly, trMtx);

            spd->createClipperLibPaths();

            res = res || clipBy(spd);
        }

        clipperPoly = clipperPoly->next();
    }

    return res;
}

void PolygonGroup::addClippingClip(PolygonGroup *pg, float *trMtx)
{
    Polygon *clipperPoly = pg->polygon();

    SimpPolyData *spd = (m_clippingClip == nullptr) ? nullptr : clippingFindLastClipData();

    while (clipperPoly != nullptr) {
        if (clipperPoly->clippingClip()) {

            SimpPolyData *s = Polygon::createSimpPolyData(clipperPoly, trMtx);

            if (!clippingOverlapTest(s)) {
                delete s;
            } else {
                if (spd == nullptr)
                    m_clippingClip = s;
                else
                    spd->m_next = s;
                spd = s;
                s->createClipperLibPaths();
            }
        }

        clipperPoly = clipperPoly->next();
    }
}

bool PolygonGroup::clip()
{
    if (m_clippingClip == nullptr)
        return false;


    bool res = false;

    SimpPolyData *spd = m_clippingClip;


    while (spd != nullptr) {

        res = res || clipBy(spd);

        spd = spd->m_next;
    }

    clippingClearClipData();

    return res;
}

bool PolygonGroup::slice(Polygon *poly, int gridSize)
{
//    Log()<<__FUNCTION__<<" start";

    if (poly->m_polyGroup != this)
        return false;

//    Log()<<__FUNCTION__<<" own poly";

    BBox *bbox = poly->bbox();
    if (!bbox->isOk())
        poly->updateBBox();

    float w = bbox->width();
    float h = bbox->height();

    float aspect = h / w;

//    Log()<<__FUNCTION__<<"bbox x: "<<bbox->xmin<<" : "<<bbox->xmax;
//    Log()<<__FUNCTION__<<"bbox y: "<<bbox->ymin<<" : "<<bbox->ymax;

    int gw, gh;
    if (aspect < 1) {
        gw = gridSize;
        gh = static_cast<int>((static_cast<float>(gridSize) * aspect) + 0.5f);
    } else {
        gh = gridSize;
        gw = static_cast<int>((static_cast<float>(gridSize) / aspect) + 0.5f);
    }

//    Log()<<__FUNCTION__<<"grid: "<<gw<<" x "<<gh;

    float ex = w * 0.05f / 2.0f;
    float ey = h * 0.05f / 2.0f;

    float dx = (w + ex + ex) / gw;
    float dy = (h + ey + ey) / gh;


    SimpPolyData * spd = new SimpPolyData(1);
    spd->m_outline.alloc4Points();

    spd->m_outline.set(0, bbox->xmin - ex*2, bbox->ymin - ey * 2);
    spd->m_outline.set(1, bbox->xmax + ex*2, bbox->ymin - ey * 2);
    spd->m_outline.set(2, bbox->xmax + ex*2, bbox->ymax + ey * 2);
    spd->m_outline.set(3, bbox->xmin - ex*2, bbox->ymax + ey * 2);

    spd->updateBBox();
    spd->m_outline.createClipperLibPath();

    spd->m_holes[0].alloc4Points();

    std::vector<Polygon *> newPolyVec;

    bool accRes = false;

    SimpContourData &cd = spd->m_holes[0];

    float x = bbox->xmin - ex;

    while ( x < bbox->xmax) {
        float y = bbox->ymin - ey;
        while (y < bbox->ymax) {
            cd.set(3, x, y);
            cd.set(2, x + dx, y);
            cd.set(1, x + dx, y + dy);
            cd.set(0, x, y + dy);


//            Log()<<__FUNCTION__<<"0: "<<cd.getX(0)<<":"<<cd.getY(0);
//            Log()<<__FUNCTION__<<"1: "<<cd.getX(1)<<":"<<cd.getY(1);
//            Log()<<__FUNCTION__<<"2: "<<cd.getX(2)<<":"<<cd.getY(2);
//            Log()<<__FUNCTION__<<"3: "<<cd.getX(3)<<":"<<cd.getY(3);

            cd.createClipperLibPath();

            bool res = poly->clipBy(spd, newPolyVec);

            accRes = accRes || res;


//            Log()<<"new vec size: "<<newPolyVec.size()<<" res: "<<res;

            y += dy;
        }
        x += dx;
    }

    delete spd;

//    Log()<<__FUNCTION__<<" accRes "<<accRes<<" newPolyVecSize: "<<newPolyVec.size();


    if (newPolyVec.size() > 0 && accRes) {
        deletePolygon(poly);
        addPolygons(newPolyVec);
        return true;
    }

    return false;
}

void PolygonGroup::deletePolygons()
{
    while (m_polygon != 0) {
        Polygon *n = m_polygon->next();
        delete m_polygon;
        m_polygon = n;
    }
}

bool PolygonGroup::clipBy(t2d2::SimpPolyData *spd)
{
    std::vector<t2d2::Polygon*> oldPolyVec;
    std::vector<t2d2::Polygon*> newPolyVec;


    Polygon *poly = m_polygon;

    bool accRes = false;

    while (poly != 0) {
        if (!poly->clippingSubj()) {
            poly = poly->next();
            continue;
        }

        bool res = poly->clipBy(spd, newPolyVec);

        if (res)
            oldPolyVec.push_back(poly);

        accRes = accRes || res;

        poly = poly->next();
    }

    if (!accRes)
        return false;

    size_t c = oldPolyVec.size();

    for(size_t i = 0; i < c; i++) {
        Polygon *poly = oldPolyVec[i];
        deletePolygon(poly);
    }

    addPolygons(newPolyVec);

    return true;
}

void PolygonGroup::clippingClearClipData()
{
    SimpPolyData *spd = m_clippingClip;

    while (spd != nullptr) {
        SimpPolyData *s = spd;
        spd = s->m_next;
        delete s;
    }

    m_clippingClip= nullptr;
}

SimpPolyData *PolygonGroup::clippingFindLastClipData()
{
    SimpPolyData *spd = m_clippingClip;

    while (spd != nullptr) {
        if (spd->m_next == nullptr)
            return spd;
        spd = spd->m_next;
    }
    return nullptr;
}

bool PolygonGroup::clippingOverlapTest(SimpPolyData *spd)
{
    Polygon* poly = m_polygon;

    while (poly != nullptr) {

        if (poly->clippingSubj()) {

            BBox *b = poly->bbox();

            if (!b->isOk())
                poly->updateBBox();

            if (BBox::overlap(b, spd->m_bbox))
                return true;
        }

        poly = poly->next();
    }

    return false;
}

void PolygonGroup::clippingAddClipToClipper(ClipperLib::Clipper &cl, SimpPolyData *spd)
{
    ClipperLib::Path clOutline;
    Contour::makeClipperLibPath(clOutline, spd->m_outline);
    cl.AddPath(clOutline, ClipperLib::ptClip, true);

    for(int i = 0; i < spd->m_hc; i++) {
        ClipperLib::Path clHole;
        Contour::makeClipperLibPath(clHole, spd->m_holes[i]);
        cl.AddPath(clHole, ClipperLib::ptClip, true);
    }
}

void PolygonGroup::clippingAddSubjToClipper(ClipperLib::Clipper &cl, Polygon *poly)
{
    ClipperLib::Path clOutine;
    poly->m_outline->makeClipperLibPath(clOutine);
    cl.AddPath(clOutine, ClipperLib::ptSubject, true);

    size_t hc = poly->m_holes.size();
    for(size_t i = 0; i < hc; i++) {
        Contour *hole = poly->m_holes[i];
        ClipperLib::Path clHole;
        hole->makeClipperLibPath(clHole);
        cl.AddPath(clHole, ClipperLib::ptSubject, true);
    }
}
