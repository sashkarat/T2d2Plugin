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
    m_valid = false;

    Polygon *poly = m_pg->polygon();

    while(poly != nullptr) {
        if (!check(poly)) {
            poly = poly->next();
            continue;
        }
        poly->validate (true);
        m_valid = m_valid || poly->isValid();
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

        unsigned int smi = brdrs->border(i)->subMeshIndex();
        if ( smi >= m_subMeshNumber)
            continue;

        Polygon *poly = m_pg->polygon();
        while (poly != nullptr) {

            allocContourBorderData(poly->outline(), mask, smi);

            int hc = poly->holesCount();

            for(int hi = 0; hi < hc; hi++)
                allocContourBorderData(poly->hole(hi), mask, smi);

            poly = poly->next();
        }
    }
}

void MeshCash::allocContourBorderData(Contour *cntr, int mask, unsigned int smi)
{
    int cm = cntr->length()-1;

    bool prevState = false;

    //over the contour loop (not including a last point)
    for(int i = 0; i < cm; i++) {
        t2d2::Point *p0 = cntr->getPoint(i);
        t2d2::Point *p1 = cntr->getPoint(i+1);

        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;

        if (state) {
            if (prevState)
                m_vertexNumber += 3;
            else
                m_vertexNumber += 5;

            m_smTriangles[smi].m_triNum += 3;
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
                m_vertexNumber += 3;
            else
                m_vertexNumber += 5;

            m_smTriangles[smi].m_triNum += 3;
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

        unsigned int smi = brdr->subMeshIndex();

        if ( smi >= m_subMeshNumber)
            continue;

        Polygon *poly = m_pg->polygon();
        while (poly != nullptr) {

            setContourBorderData(poly->outline(), mask, brdr);

            int hc = poly->holesCount();

            for(int hi = 0; hi < hc; hi++)
                allocContourBorderData(poly->hole(hi), mask, smi);

            poly = poly->next();
        }
    }
}

void MeshCash::setContourBorderData(Contour *cntr, int mask, Border *brdr)
{
    int cm = cntr->length()-1;

    bool prevState = false;

    int p0_idx, p1_idx, p2_idx, p3_idx, p4_idx;

    float zValue = cntr->getPoly()->zValue() + brdr->zOffset();
    float offset = brdr->offset();
    float width = brdr->width();

    //over the contour loop (not including a last point)
    for(int i = 0; i < cm; i++) {
        t2d2::Point *p0 = cntr->getPoint(i);
        t2d2::Point *p1 = cntr->getPoint(i+1);
        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;

        if (state) {
            if (prevState) {
//                2 new points;
                p0_idx = m_verticesDone - 2;
                p3_idx = m_verticesDone - 1;
                p4_idx = m_verticesDone;
                p1_idx = m_verticesDone + 1;
                p2_idx = m_verticesDone + 2;

                setBorderVertex(p1_idx, p1, zValue, offset, 0, brdr);
                setBorderVertex(p2_idx, p1, zValue, offset + width, 1, brdr);

                setBorderSegmentMidVertex(p4_idx, p2_idx, p3_idx, 1, brdr);

                m_verticesDone += 3;
            } else {
//                  4 new points
                p0_idx = m_verticesDone;
                p3_idx = m_verticesDone + 1;
                p4_idx = m_verticesDone + 2;
                p1_idx = m_verticesDone + 3;
                p2_idx = m_verticesDone + 4;


                setBorderVertex(p0_idx, p0, zValue, offset, 0, brdr);
                setBorderVertex(p3_idx, p0, zValue, offset + width, 1, brdr);

                setBorderVertex(p1_idx, p1, zValue, offset, 0, brdr);
                setBorderVertex(p2_idx, p1, zValue, offset + width, 1, brdr);

                setBorderSegmentMidVertex(p4_idx, p2_idx, p3_idx, 1, brdr);

                m_verticesDone += 5;

            }

            setBorderSegmentTriangle(p0_idx, p4_idx, p3_idx, brdr->subMeshIndex());
            setBorderSegmentTriangle(p4_idx, p0_idx, p1_idx, brdr->subMeshIndex());
            setBorderSegmentTriangle(p2_idx, p4_idx, p1_idx, brdr->subMeshIndex());
        }
        prevState = state;
    }

    //at the last point (the last edge) iteration to close the contour
    {
        t2d2::Point *p0 = cntr->getPoint(cm);
        t2d2::Point *p1 = cntr->getPoint(0);
        bool state = (p0->m_borderFlags & p1->m_borderFlags & mask) != 0;

        if (state) {

            float dx = p1->x - p0->x;
            float dy = p1->y - p0->y;

            float d = sqrtf(dx*dx + dy * dy);

            float pp = p1->m_position;

            p1->m_position = d + p0->m_position;

            if (prevState) {
//                2 new points;
                p0_idx = m_verticesDone - 2;
                p3_idx = m_verticesDone - 1;
                p4_idx = m_verticesDone;
                p1_idx = m_verticesDone + 1;
                p2_idx = m_verticesDone + 2;

                setBorderVertex(p1_idx, p1, zValue, offset, 0, brdr);
                setBorderVertex(p2_idx, p1, zValue, offset + width, 1, brdr);

                setBorderSegmentMidVertex(p4_idx, p2_idx, p3_idx, 1, brdr);

                m_verticesDone += 3;
            } else {
//                  4 new points
                p0_idx = m_verticesDone;
                p3_idx = m_verticesDone + 1;
                p4_idx = m_verticesDone + 2;
                p1_idx = m_verticesDone + 3;
                p2_idx = m_verticesDone + 4;


                setBorderVertex(p0_idx, p0, zValue, offset, 0, brdr);
                setBorderVertex(p3_idx, p0, zValue, offset + width, 1, brdr);

                setBorderVertex(p1_idx, p1, zValue, offset, 0, brdr);
                setBorderVertex(p2_idx, p1, zValue, offset + width, 1, brdr);

                setBorderSegmentMidVertex(p4_idx, p2_idx, p3_idx, 1, brdr);

                m_verticesDone += 5;
            }

            setBorderSegmentTriangle(p0_idx, p4_idx, p3_idx, brdr->subMeshIndex());
            setBorderSegmentTriangle(p4_idx, p0_idx, p1_idx, brdr->subMeshIndex());
            setBorderSegmentTriangle(p2_idx, p4_idx, p1_idx, brdr->subMeshIndex());

            p1->m_position = pp;
        }
        prevState = state;
    }
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

    float u = p->m_position - dx;

    m_uv[ti+0] = u * brdr->uScale() + brdr->uOffset();
    m_uv[ti+1] = v * brdr->vScale() + brdr->vOffset();
}

void MeshCash::setBorderSegmentMidVertex(int idx, int p2_idx, int p3_idx, float v, Border *brdr)
{
    int vi = idx *3;

    int p2i = p2_idx * 3;
    int p3i = p3_idx * 3;

    m_vertices[vi + 0] = (m_vertices[p2i] - m_vertices[p3i]) / 2.0f + m_vertices[p3i];
    m_vertices[vi + 1] = (m_vertices[p2i+1] - m_vertices[p3i+1]) / 2.0f + m_vertices[p3i+1];
    m_vertices[vi + 2] = m_vertices[p3i + 2];

    int ti = idx*2;

    p2_idx *= 2;
    p3_idx *= 2;

    float u = (m_uv[p3_idx] - m_uv[p2_idx]) / 2.0f + m_uv[p2_idx];

    m_uv[ti+0] = u;
    m_uv[ti+1] = v * brdr->vScale() + brdr->vOffset();
}

void MeshCash::setBorderSegmentTriangle(int idxA, int idxB, int idxC, int smi)
{
    SubMeshTriangles &trs = m_smTriangles[smi];

    int idx = trs.m_triDone * 3;

    trs.m_tri[idx + 0] = idxA;
    trs.m_tri[idx + 1] = idxB;
    trs.m_tri[idx + 2] = idxC;

    trs.m_triDone += 1;
}

