#include "mcash.h"

using namespace t2d2;

unsigned int MCash::triangleNumber(unsigned int smi)
{
    if (m_smTriangles == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no triangulation data";
        return 0;
    }
    if (smi >= m_subMeshNumber) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return 0;
    }
    return m_smTriangles[smi].m_triNum;
}

void MCash::cpyVertices(float *out)
{
    if (m_vertices == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no vertices data";
        return;
    }
    memcpy(out, m_vertices, sizeof(float) * m_vertexNumber * m_stride);
}

void MCash::cpyUv(float *out)
{
    if (m_uv == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no uv data";
        return;
    }

    memcpy(out, m_uv, sizeof(float) * m_vertexNumber * 2);
}

void MCash::cpyIndices(unsigned int smi, int *out)
{
    if (m_smTriangles == nullptr) {
        Log(ltError)<<__FUNCTION__<<"no triangulation data";
        return;
    }
    if (smi >= m_subMeshNumber) {
        Log(ltWarning)<<__FUNCTION__<<"index out of range";
        return;
    }

    memcpy(out, m_smTriangles[smi].m_tri, sizeof(int) * m_smTriangles[smi].m_triNum * 3);
}

MCash::MCash (PolygonGroup *pg) :
    m_stride(0),
    m_vertexNumber(0),
    m_vertices(nullptr),
    m_uv(nullptr),
    m_subMeshNumber(0),
    m_smTriangles(nullptr),
    m_pg(pg),
    m_valid(false)
{

}

MCash::~MCash()
{
    free();
}

void MCash::allocate(t2d2::MCashContentOptions mcocOpt, t2d2::MCashStageOptions mcosOpt, int stride, int subMeshNum)
{
    free();

    m_mcocOpt = mcocOpt;
    m_mcosOpt = mcosOpt;

    m_stride = stride;
    m_subMeshNumber = subMeshNum;

    if (m_mcosOpt & mcosValidate) {
        if (!validate ())
            return;
        m_valid = true;
    }

    allocData();

    setData();

}

void MCash::free()
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

bool MCash::validate()
{
    bool res = false;
    bool withHoles = ((m_mcocOpt & mcocHoles) != 0);

    Polygon *poly = m_pg->polygon();

    while(poly != nullptr) {
        if (!contentCheck(poly)) {
            poly = poly->next();
            continue;
        }
        poly->validate (withHoles);
        res = res || poly->isValid();
        poly = poly->next();
    }

    return res;
}

void MCash::allocData()
{
    if (((m_mcosOpt & mcosAllocTriangles) != 0) && ((m_mcosOpt & mcosTraingulate) != 0))
        m_smTriangles = new SubMeshTriangles[m_subMeshNumber];

    Polygon *poly = m_pg->polygon();
    while (poly != nullptr) {
        if (!contentCheck (poly)) {
            poly = poly->next();
            continue;
        }

        if (m_mcosOpt & mcosAllocVertices)
            allocPolyVertexData (poly);

        allocPolyTrianglesData(poly);

        poly = poly->next();
    }

    if (m_mcosOpt & mcosAllocVertices) {
        m_vertices = new float [m_vertexNumber * m_stride];
        if (m_mcosOpt & mcosProcessUV)
            m_uv = new float [m_vertexNumber * 2];
    }
}

void MCash::allocPolyVertexData(Polygon *poly)
{
    if (!poly->isValid())
        return;

    allocContourVertexData(poly->outline());

    if (m_mcocOpt & mcocHoles) {
        for(int i = 0; i < poly->holesCount(); i++)
            allocContourVertexData(poly->hole(i));
    }
}

void MCash::allocContourVertexData(Contour *contour)
{
    if (!contour->isValid())
        return;

    contour->setCashOffset(m_vertexNumber);
    m_vertexNumber += contour->length();
}

void MCash::allocPolyTrianglesData(Polygon *poly)
{
    if (m_mcosOpt & mcosTraingulate) {

        poly->triangulate ((m_mcosOpt & mcosUpdateArea) != 0,
                           (m_mcosOpt & mcosAllocTriangles) != 0,
                           (m_mcocOpt & mcocHoles) != 0);

        if (m_mcosOpt & mcosAllocTriangles) {
            poly->setCashTriOffset(m_smTriangles[poly->subMeshIndex()].m_triNum);
            m_smTriangles[poly->subMeshIndex()].m_triNum += poly->triNumber();
        }
    }
}

void MCash::setData()
{
    Polygon *poly = m_pg->polygon();

    while (poly != nullptr) {
        if (!contentCheck (poly)) {
            poly = poly->next();
            continue;
        }
        if (m_mcosOpt & mcosAllocVertices)
            setPolyVerticesData (poly);
        if (m_mcosOpt & mcosTriProcessing)
            setTrianglesData(poly);
        poly = poly->next();
    }
}

void MCash::setPolyVerticesData(Polygon *poly)
{
    if (!poly->isValid())
        return;

    setContourVerticesData (poly->outline());

    if (m_mcocOpt & mcocHoles) {
        for(int i = 0; i < poly->holesCount(); i++)
            setContourVerticesData (poly->hole(i));
    }
}

void MCash::setContourVerticesData(Contour *contour)
{
    if (!contour->isValid())
        return;

    UvProjection *uvProj = contour->getPoly()->getUvProjection();

    for(unsigned int i = 0; i <contour->length(); i++) {
        t2d2::Point * p = contour->getPoint(i);
        p->m_index = contour->getCashOffset() + i;
        int idx = p->m_index * m_stride;
        m_vertices[idx+0] = p->x;
        m_vertices[idx+1] = p->y;
        if (m_stride >= 3)
            m_vertices [idx + 2] = contour->getPoly()->zValue();

        if (m_mcosOpt & mcosProcessUV) {
            idx = p->m_index * 2;
            float u, v;
            uvProj->computeUV(p, u, v);
            m_uv[idx + 0] = u;
            m_uv[idx + 1] = v;
        }
    }
}

void MCash::setTrianglesData(Polygon *poly)
{
    unsigned int smi = poly->subMeshIndex();

    if (smi >= m_subMeshNumber)
        return;

    int offset = poly->cashTriOffset();

    SubMeshTriangles &trs = m_smTriangles[smi];
    if (trs.m_tri == nullptr)
        trs.m_tri = new int [trs.m_triNum * 3];

    for(unsigned int i = 0; i < poly->triNumber(); i++) {
        Polygon::Triangle *t = poly->tri(i);

        int idx = (offset + i) * 3;

        t2d2::Point *p0 = t->points[2];
        t2d2::Point *p1 = t->points[1];
        t2d2::Point *p2 = t->points[0];

        trs.m_tri[idx + 0] = p0->m_index;
        trs.m_tri[idx + 1] = p1->m_index;
        trs.m_tri[idx + 2] = p2->m_index;
    }
}

bool MCash::contentCheck(Polygon *poly)
{
    if (m_mcocOpt & t2d2::mcocAny)
        return true;

    if (poly->genMesh()  && (m_mcocOpt & t2d2::mcocMesh))
        return (poly->subMeshIndex() < m_subMeshNumber);

    if (poly->genCollider() && (m_mcocOpt & t2d2::mcocCollider))
        return true;

    return false;
}
