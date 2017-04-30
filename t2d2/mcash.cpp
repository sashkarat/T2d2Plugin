#include "mcash.h"

using namespace t2d2;

unsigned int MCash::stride() const
{
    return m_stride;
}

unsigned int MCash::vertexNumber() const
{
    return m_vertexNumber;
}

unsigned int MCash::subMeshNumber() const
{
    return m_subMeshNumber;
}

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

MCash::MCash() :
    m_stride(0),
    m_vertexNumber(0),
    m_vertices(nullptr),
    m_uv(nullptr),
    m_subMeshNumber(0),
    m_smTriangles(nullptr)
{

}

MCash::~MCash()
{
    free();
}

void MCash::allocate(int stride, PolygonGroup *pg, int subMeshNum)
{
    free();

    m_stride = stride;
    m_allocPg = pg;
    m_subMeshNumber = subMeshNum;
    m_smTriangles = new SubMeshTriangles[m_subMeshNumber];

    Polygon *poly = m_allocPg->polygon();
    while (poly != nullptr) {
        allocAddPolygon(poly);
        poly = poly->next();
    }

    m_vertices = new float [m_vertexNumber * m_stride];
    m_uv = new float [m_vertexNumber * 2];

    poly = m_allocPg->polygon();
    while (poly != nullptr) {
        allocSetVerticesData(poly);
        allocSetTrianglesData(poly);
        poly = poly->next();
    }


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
}

void MCash::allocAddContour(Contour *contour)
{
    contour->setCashOffset(m_vertexNumber);
    m_vertexNumber += contour->length();
}

void MCash::allocAddPolygon(Polygon *poly)
{
    if (!poly->genMesh())
        return;

    if (poly->subMeshIndex() >= m_subMeshNumber)
        return;

    allocAddContour(poly->contour());

    for(int i = 0; i < poly->holesCount(); i++)
        allocAddContour(poly->hole(i));

    poly->setCashTriOffset(m_smTriangles[poly->subMeshIndex()].m_triNum);
    m_smTriangles[poly->subMeshIndex()].m_triNum += poly->triNumber();
}

void MCash::allocSetVerticesData(Polygon *poly)
{
    if (!poly->genMesh())
        return;

    if (poly->subMeshIndex() >= m_subMeshNumber)
        return;

    allocSetVerticesData (poly->contour());

    for(int i = 0; i < poly->holesCount(); i++)
        allocSetVerticesData (poly->hole(i));
}

void MCash::allocSetVerticesData(Contour *contour)
{
    for(unsigned int i = 0; i <contour->length(); i++) {
        t2d2::Point * p = contour->getPoint(i);
        p->m_index = contour->getCashOffset() + i;
        int idx = p->m_index * m_stride;
        m_vertices[idx+0] = p->x;
        m_vertices[idx+1] = p->y;

        idx = p->m_index * 2;

        //TODO: apply uv mapping
        m_uv[idx + 0] = 0;
        m_uv[idx + 1] = 1;

        unsigned int st = m_stride;
        while (st > 2)
            m_vertices [idx + st-- - 1] = contour->getPoly()->zValue();
    }
}

void MCash::allocSetTrianglesData(Polygon *poly)
{
    int smi = poly->subMeshIndex();
    int offset = poly->cashTriOffset();

    SubMeshTriangles &trs = m_smTriangles[smi];
    if (trs.m_tri == nullptr)
        trs.m_tri = new int [trs.m_triNum * 3];

    for(unsigned int i = 0; i < poly->triNumber(); i++) {
        p2t::Triangle *t = poly->tri(i);

        int idx = (offset + i) * 3;

        t2d2::Point *p0 = dynamic_cast<t2d2::Point*>(t->GetPoint(0));
        t2d2::Point *p1 = dynamic_cast<t2d2::Point*>(t->GetPoint(1));
        t2d2::Point *p2 = dynamic_cast<t2d2::Point*>(t->GetPoint(2));

        trs.m_tri[idx + 0] = p0->m_index;
        trs.m_tri[idx + 1] = p1->m_index;
        trs.m_tri[idx + 2] = p2->m_index;
    }
}
