#include "meshcash.h"

using namespace  t2d2;

MeshCash::MeshCash(PolygonGroup *pg) :
    m_vertexNumber(0),
    m_vertices(nullptr),
    m_uv(nullptr),
    m_subMeshNumber(0),
    m_smTriangles(nullptr),
    m_pg(pg),
    m_valid(false)
{

}

MeshCash::~MeshCash()
{
    free();
}

bool MeshCash::build(unsigned int subMeshNumber)
{
    free();

    m_subMeshNumber = subMeshNumber;

    if (!validate())
        return false;

    allocate();

    set();

    return false;
}

unsigned int MeshCash::triangleNumber(unsigned int smi) const
{
    if (m_smTriangles == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no triangulation data";
        return 0;
    }
    if (smi >= m_subMeshNumber) {
        Log(ltWarning)<<__FUNCTION__<<"smi out of range";
        return 0;
    }
    return m_smTriangles[smi].m_triNum;
}

void MeshCash::cpyVertices(float *out)
{
    if (m_vertices == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no vertices data";
        return;
    }
    memcpy(out, m_vertices, sizeof(float) * m_vertexNumber * 3);
}

void MeshCash::cpyUv(float *out)
{
    if (m_uv == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no uv data";
        return;
    }

    memcpy(out, m_uv, sizeof(float) * m_vertexNumber * 2);
}

void MeshCash::cpyIndices(unsigned int smi, int *out)
{
    if (m_smTriangles == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no triangulation data";
        return;
    }
    if (smi >= m_subMeshNumber) {
        Log(ltWarning)<<__FUNCTION__<<"smi out of range";
        return;
    }

    memcpy(out, m_smTriangles[smi].m_tri, sizeof(int) * m_smTriangles[smi].m_triNum * 3);
}

void MeshCash::free()
{
    if (m_vertices)
        delete [] m_vertices;

    if (m_uv)
        delete [] m_uv;

    if (m_smTriangles)
        delete [] m_smTriangles;

    m_vertexNumber = 0;
    m_vertices = nullptr;
    m_uv = nullptr;
    m_subMeshNumber = 0;
    m_smTriangles = nullptr;
    m_valid = false;
}

bool MeshCash::validate()
{
    m_valid = true;

    Polygon *poly = m_pg->polygon();

    while(poly != nullptr) {
        if (!check(poly)) {
            poly = poly->next();
            continue;
        }
        if (!poly->validate (true)) {
            m_valid = false;
            return m_valid;
        }
        poly = poly->next();
    }
    return m_valid;
}

void MeshCash::allocate()
{

    m_smTriangles = new SubMeshTriangles[m_subMeshNumber];

    Polygon *poly = m_pg->polygon();
    while (poly != nullptr) {
        if (!check (poly)) {
            poly = poly->next();
            continue;
        }

        allocVertices (poly);

        allocTriangles(poly);

        poly = poly->next();
    }

    allocBorderData();

    if (m_vertexNumber > 0) {
        m_vertices = new float [m_vertexNumber * 3];
        m_uv = new float [m_vertexNumber * 2];
    }

    m_verticesDone = 0;

    for(unsigned int i = 0; i < m_subMeshNumber; i++)
        m_smTriangles[i].allocate();
}

void MeshCash::set()
{
    Polygon *poly = m_pg->polygon();

    while (poly != nullptr) {
        if (!check (poly)) {
            poly = poly->next();
            continue;
        }

        setVertices (poly);

        setTriangles(poly);

        poly = poly->next();
    }

    setBorderData();
}

bool MeshCash::check(Polygon *poly)
{
    if (poly->genMesh())
        return (poly->subMeshIndex() < m_subMeshNumber);
    return false;
}

void MeshCash::allocVertices(Polygon *poly)
{
    if (!poly->isValid())
        return;

    allocContour(poly->outline());

    for(int i = 0; i < poly->holesCount(); i++)
        allocContour(poly->hole(i));
}

void MeshCash::allocBorderData()
{
    Borders *brdrs = m_pg->borders();
    int mask = 1;

    for(int i = 0; i < 32; i++, mask <<= 1) {

        if (! brdrs->borderEnabled(i))
            continue;

        Border *brdr = brdrs->border(i);

        if ( brdr->subMeshIndex() >= m_subMeshNumber)
            continue;

        Polygon *poly = m_pg->polygon();
        while (poly != nullptr) {

            allocContourBorderData(poly->outline(), mask, brdr);

            int hc = poly->holesCount();

            for(int hi = 0; hi < hc; hi++)
                allocContourBorderData(poly->hole(hi), mask, brdr);

            poly = poly->next();
        }
    }
}

void MeshCash::allocContourBorderData(Contour *cntr, int mask, Border *brdr)
{
    int smi = brdr->subMeshIndex();

    int cm = cntr->length()-1;

    bool prevState = false;

    int av = brdr->triOrder() * 2 - 1;

    //over the contour loop (not including a last point)
    for(int i = 0; i < cm; i++) {
        t2d2::Point *p0 = cntr->getPoint(i);
        t2d2::Point *p1 = cntr->getPoint(i+1);

        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;



        if (state) {
            if (prevState)
                m_vertexNumber += 2 + av;
            else
                m_vertexNumber += 4 + av;

            m_smTriangles[smi].m_triNum += av + 2;
        }
        prevState = state;
    }

    //at the last point (the last edge) iteration to close the contour
    {
        t2d2::Point *p0 = cntr->getPoint(cm);
        t2d2::Point *p1 = cntr->getPoint(0);

        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;

        if (state) {
            if (prevState)
                m_vertexNumber += 2 + av;
            else
                m_vertexNumber += 4 + av;

            m_smTriangles[smi].m_triNum += av + 2;
        }
        prevState = state;
    }
}

void MeshCash::allocTriangles(Polygon *poly)
{
    poly->triangulate ();

    poly->setCashTriOffset(m_smTriangles[poly->subMeshIndex()].m_triNum);

    m_smTriangles[poly->subMeshIndex()].m_triNum += poly->triNumber();
}

void MeshCash::allocContour(Contour *contour)
{
    if (!contour->isValid())
        return;

    contour->setCashOffset(m_vertexNumber);
    m_vertexNumber += contour->length();
}

void MeshCash::setVertices(Polygon *poly)
{
    if (!poly->isValid())
        return;

    setVertices (poly->outline());

    for(int i = 0; i < poly->holesCount(); i++)
        setVertices (poly->hole(i));
}

void MeshCash::setVertices(Contour *contour)
{
    if (!contour->isValid())
        return;

    UvProjection *uvProj = contour->getPoly()->getUvProjection();

    unsigned int cl = contour->length();

    for(unsigned int i = 0; i < cl; i++) {
        t2d2::Point * p = contour->getPoint(i);
        p->m_index = contour->getCashOffset() + i;
        int idx = p->m_index * 3;

        m_vertices [idx + 0] = p->x;
        m_vertices [idx + 1] = p->y;
        m_vertices [idx + 2] = contour->getPoly()->zValue();

        idx = p->m_index * 2;
        float u, v;
        uvProj->computeUV(p, u, v);
        m_uv[idx + 0] = u;
        m_uv[idx + 1] = v;
    }

    m_verticesDone += cl;
}

void MeshCash::setTriangles(Polygon *poly)
{
    unsigned int smi = poly->subMeshIndex();

    if (smi >= m_subMeshNumber)
        return;

    int offset = poly->cashTriOffset();

    SubMeshTriangles &trs = m_smTriangles[smi];

    unsigned int tn = poly->triNumber();

    for(unsigned int i = 0; i < tn; i++) {
        Polygon::Triangle *t = poly->tri(i);

        int idx = (offset + i) * 3;

        t2d2::Point *p0 = t->points[2];
        t2d2::Point *p1 = t->points[1];
        t2d2::Point *p2 = t->points[0];

        trs.m_tri[idx + 0] = p0->m_index;
        trs.m_tri[idx + 1] = p1->m_index;
        trs.m_tri[idx + 2] = p2->m_index;
    }

    trs.m_triDone += tn;
}

void MeshCash::setBorderData()
{
    Borders *brdrs = m_pg->borders();
    int mask = 1;

    for(int i = 0; i < 32; i++, mask <<= 1) {
        if (! brdrs->borderEnabled(i))
            continue;

        Border *brdr = brdrs->border(i);

        if (brdr->subMeshIndex() >= m_subMeshNumber)
            continue;

        Polygon *poly = m_pg->polygon();
        while (poly != nullptr) {

            setContourBorderData(poly->outline(), mask, brdr);

            int hc = poly->holesCount();

            for(int hi = 0; hi < hc; hi++)
                setContourBorderData(poly->hole(hi), mask, brdr);

            poly = poly->next();
        }
    }
}

void MeshCash::setContourBorderData(Contour *cntr, int mask, Border *brdr)
{
    int cm = cntr->length()-1;

    bool prevState = false;

    float zValue = cntr->getPoly()->zValue() + brdr->zOffset();

    //over the contour loop (not including the last point)
    for(int i = 0; i < cm; i++) {
        t2d2::Point *p0 = cntr->getPoint(i);
        t2d2::Point *p1 = cntr->getPoint(i+1);

        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;

        if (state)
            setContourBorderSegmentData(p0, p1, prevState, zValue, brdr );

        prevState = state;
    }

    //at the last point (the last edge) iteration to close the contour
    {
        t2d2::Point *p0 = cntr->getPoint(cm);
        t2d2::Point *p1 = cntr->getPoint(0);

        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;
        if (state) {

            float pp =p1->m_position;

            float dx = p1->x - p0->x;
            float dy = p1->y - p0->y;

            float d = sqrtf(dx * dx + dy * dy);

            p1->m_position =  p0->m_position + d;

            setContourBorderSegmentData(p0, p1, prevState, zValue, brdr );

            p1->m_position = pp;
        }

        prevState = state;
    }
}

void MeshCash::setContourBorderSegmentData(Point *p0, Point *p1, bool prevState, float zValue, Border *brdr)
{
    int p0_idx, p1_idx, p2_idx, p3_idx;

    int av = brdr->triOrder() * 2 - 1;

    int dv = 0;

    int io = 0;

    if (prevState) {
        //                2 new points;
        p0_idx = m_verticesDone - 2;
        p3_idx = m_verticesDone - 1;

        p1_idx = m_verticesDone + av;
        p2_idx = m_verticesDone + av + 1;

        setBorderVertex(p1_idx, p1, zValue, brdr->offset(), 0, brdr);
        setBorderVertex(p2_idx, p1, zValue, brdr->offset() + brdr->width(), 1, brdr);

         dv =  2 + av;
    } else {
        //                  4 new points
        p0_idx = m_verticesDone;
        p3_idx = m_verticesDone + 1;
        p1_idx = m_verticesDone + av + 2;
        p2_idx = m_verticesDone + av + 3;


        setBorderVertex(p0_idx, p0, zValue, brdr->offset(), 0, brdr);
        setBorderVertex(p3_idx, p0, zValue, brdr->offset() + brdr->width(), 1, brdr);

        setBorderVertex(p1_idx, p1, zValue, brdr->offset(), 0, brdr);
        setBorderVertex(p2_idx, p1, zValue, brdr->offset() + brdr->width(), 1, brdr);

        io = 2;

        dv = 4 + av;
    }


    setBorderSegmentMidVertices(p0_idx, p1_idx, p2_idx, p3_idx, io, brdr);

    m_verticesDone += dv;

//    setBorderSegmentTriangle(p0_idx, p4_idx, p3_idx, brdr->subMeshIndex());
//    setBorderSegmentTriangle(p4_idx, p0_idx, p1_idx, brdr->subMeshIndex());
//    setBorderSegmentTriangle(p2_idx, p4_idx, p1_idx, brdr->subMeshIndex());
}

void MeshCash::setBorderVertex(int idx, Point *p, float z, float offset, float v, Border *brdr)
{
    idx;

    int vi = idx * 3;

    m_vertices[vi + 0] = p->x + p->m_miterX * offset / p->m_dotPr;
    m_vertices[vi + 1] = p->y + p->m_miterY * offset / p->m_dotPr;;
    m_vertices[vi + 2] = z;

    int ti = idx * 2;

    float xn = p->x + p->m_normX * offset;
    float dx = xn - m_vertices[vi + 0];

    float u = p->m_position - dx*p->m_dotPr;

    m_uv[ti+0] = u * brdr->uScale() + brdr->uOffset();
    m_uv[ti+1] = v * brdr->vScale() + brdr->vOffset();
}

void MeshCash::setBorderSegmentMidVertices(int p0_idx, int p1_idx, int p2_idx, int p3_idx, int io, Border *brdr)
{
    int p0i = p0_idx * 3;
    int p1i = p1_idx * 3;
    int p2i = p2_idx * 3;
    int p3i = p3_idx * 3;

    int topEdgeNum = brdr->triOrder() + 1;
    int btmEdgeNum = brdr->triOrder();

    float tdx = (m_vertices[p2i] - m_vertices[p3i]) / topEdgeNum;
    float tdy = (m_vertices[p2i + 1] - m_vertices[p3i + 1]) / topEdgeNum;


    float bdx = (m_vertices[p1i] - m_vertices[p0i]) / btmEdgeNum;
    float bdy = (m_vertices[p1i+1] - m_vertices[p0i+1]) / btmEdgeNum;

    int u0i = p0_idx*2;
    int u1i = p1_idx*2;
    int u2i = p2_idx*2;
    int u3i = p3_idx*2;

    float tdu = (m_uv[u2i] - m_uv[u3i]) / topEdgeNum;
    float bdu = (m_uv[u1i] - m_uv[u0i]) / btmEdgeNum;

    float tx = m_vertices[p3i];
    float ty = m_vertices[p3i+1];
    float bx = m_vertices[p0i];
    float by = m_vertices[p0i+1];
    float tu = m_uv[u3i];
    float bu = m_uv[u0i];


    int to = brdr->triOrder() - 1;

    for(int i = 0; i < to; i++) {

        tx += tdx;
        ty += tdy;
        bx += bdx;
        by += bdy;

        tu += tdu;
        bu += bdu;

        int idx = (io + i * 2 + m_verticesDone) * 3;

        m_vertices[idx] = tx;
        m_vertices[idx+1] = ty;
        m_vertices[idx+2] = m_vertices[p1i + 2];

        idx = (io + i * 2 + m_verticesDone) * 2;

        m_uv[idx] = tu;
        m_uv[idx+1] = brdr->vScale() + brdr->vOffset();


        idx = (io + i * 2 + 1 + m_verticesDone) * 3;

        m_vertices[idx] = bx;
        m_vertices[idx+1] = by;
        m_vertices[idx+2] = m_vertices[p0i + 2];

        idx = (io + i * 2 + 1 + m_verticesDone) * 2;

        m_uv[idx] = bu;
        m_uv[idx+1] = brdr->vOffset();
    }

    {
        tx += tdx;
        ty += tdy;
        tu += tdu;

        int idx = (io + to * 2 + m_verticesDone) * 3;

        m_vertices[idx] = tx;
        m_vertices[idx+1] = ty;
        m_vertices[idx+2] = m_vertices[p3i+2];

        idx = (io + to * 2 + m_verticesDone) * 2;

        m_uv[idx] = tu;
        m_uv[idx+1] = brdr->vScale() + brdr->vOffset();

    }

    setBorderSegmentTriangle(p3_idx, p0_idx, io + m_verticesDone, brdr->subMeshIndex());

//    Log()<<__FUNCTION__<<"to: "<<to;

    if (!to) {
        setBorderSegmentTriangle(io + m_verticesDone, p0_idx, p1_idx, brdr->subMeshIndex());
    } else {

        int lt = brdr->triOrder() * 2 - 2;

        setBorderSegmentTriangle(p0_idx, io + m_verticesDone, io + 1 + m_verticesDone , brdr->subMeshIndex());

        for(int ii = 0; ii < lt; ii++) {
            int idxA = io + ii + m_verticesDone;
//            Log()<<__FUNCTION__<<" ii: "<<ii<<" A"<<idxA<<" io: "<<io<<" vd: "<<m_verticesDone;
            setBorderSegmentTriangle(idxA, idxA+1, idxA+2, brdr->subMeshIndex());
        }
    }

    setBorderSegmentTriangle(io + to * 2 + m_verticesDone, p1_idx, p2_idx, brdr->subMeshIndex());
}

void MeshCash::setBorderSegmentTriangle(int idxA, int idxB, int idxC, int smi)
{

//    Log()<<__FUNCTION__<<"A: "<<idxA<<" B: "<<idxB<<" C"<<idxC;

    SubMeshTriangles &trs = m_smTriangles[smi];

    int idx = trs.m_triDone * 3;

    trs.m_tri[idx + 0] = idxA;
    trs.m_tri[idx + 1] = idxB;
    trs.m_tri[idx + 2] = idxC;

    trs.m_triDone += 1;
}

