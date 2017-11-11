#include "meshcash.h"

using namespace  t2d2;

MeshCash::MeshCash(PolygonGroup *pg) :
    m_vertexNumber(0),
    m_vertices(0),
    m_uv(0),
    m_subMeshNumber(0),
    m_smTriangles(0),
    m_pg(pg),
    m_valid(false)
{

}

MeshCash::~MeshCash()
{
    free();
}

bool MeshCash::build(unsigned int subMeshNumber, bool doValidation)
{
    free();

    m_subMeshNumber = subMeshNumber;

    if (doValidation) {
        if (!validate())
            return false;
    } else {
        Polygon *poly = m_pg->polygon();
        while (poly != 0) {
            poly->markValid();
            poly = poly->next();
        }
        m_valid = true;
    }

    allocate();

    set();

    return false;
}

unsigned int MeshCash::triangleNumber(unsigned int smi) const
{
    if (m_smTriangles == 0) {
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
    if (m_vertices == 0) {
        Log(ltError)<<__FUNCTION__<<"no vertices data";
        return;
    }
    memcpy(out, m_vertices, sizeof(float) * m_vertexNumber * 3);
}

void MeshCash::cpyUv(float *out)
{
    if (m_uv == 0) {
        Log(ltError)<<__FUNCTION__<<"no uv data";
        return;
    }

    memcpy(out, m_uv, sizeof(float) * m_vertexNumber * 2);
}

void MeshCash::cpyIndices(unsigned int smi, int *out)
{
    if (m_smTriangles == 0) {
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
    m_vertices = 0;
    m_uv = 0;
    m_subMeshNumber = 0;
    m_smTriangles = 0;
    m_valid = false;
}

bool MeshCash::validate()
{
    m_valid = true;

    Polygon *poly = m_pg->polygon();

    while(poly != 0) {
        if (!check(poly)) {
            poly = poly->next();
            continue;
        }
        if (!poly->validate (true)) {
            Log(ltError)<<__FUNCTION__<<" poly validation failed";
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
    while (poly != 0) {
        if (!check (poly)) {
            poly = poly->next();
            continue;
        }

        allocVertices (poly);

        allocTriangles(poly);

        poly = poly->next();
    }

//    allocBorderData();

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

    while (poly != 0) {
        if (!check (poly)) {
            poly = poly->next();
            continue;
        }

        setVertices (poly);

        setTriangles(poly);

        poly = poly->next();
    }

//    setBorderData();
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

void MeshCash::allocTriangles(Polygon *poly)
{
    poly->triangulate (true, false);

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

    unsigned int cl = contour->length();

    float z = contour->getPoly()->zValue();

    for(unsigned int i = 0; i < cl; i++) {
        t2d2::Point * p = contour->getPoint(i);
        p->m_index = contour->getCashOffset() + i;
        int idx = p->m_index * 3;

        m_vertices [idx + 0] = p->x;
        m_vertices [idx + 1] = p->y;
        m_vertices [idx + 2] = z;

        idx = p->m_index * 2;
        m_uv[idx + 0] = p->m_u;
        m_uv[idx + 1] = p->m_v;
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
        Triangle *t = poly->tri(i);

        int idx = (offset + i) * 3;

        t2d2::Point *p0 = t->m_p[2];
        t2d2::Point *p1 = t->m_p[1];
        t2d2::Point *p2 = t->m_p[0];


        if (p0 == nullptr || p1 == nullptr || p2 == nullptr) {
            trs.m_tri[idx + 0] = 0;
            trs.m_tri[idx + 1] = 0;
            trs.m_tri[idx + 2] = 0;
        } else {
            trs.m_tri[idx + 0] = p0->m_index;
            trs.m_tri[idx + 1] = p1->m_index;
            trs.m_tri[idx + 2] = p2->m_index;
        }
    }

    trs.m_triDone += tn;
}

