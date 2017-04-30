#ifndef MCASH_H
#define MCASH_H

#include "t2d2.h"

namespace t2d2 {

class Contour;
class Polygon;
class PolygonGroup;

class MCash
{

    class SubMeshTriangles
    {
    public:
        unsigned int m_triNum;
        int *m_tri;
        SubMeshTriangles () : m_triNum(0), m_tri(nullptr) {}
        ~SubMeshTriangles() {if (m_tri) delete [] m_tri;}
    };

    unsigned int        m_stride;
    unsigned int        m_vertexNumber;
    float*              m_vertices;
    float*              m_uv;

    unsigned int        m_subMeshNumber;
    SubMeshTriangles*   m_smTriangles;
    PolygonGroup*       m_allocPg;

public:
    MCash();
    ~MCash();

    void allocate(int stride, PolygonGroup *pg, int subMeshNum);

    unsigned int stride() const;

    unsigned int vertexNumber() const;

    unsigned int subMeshNumber() const;

    unsigned int triangleNumber(unsigned int smi);

    void cpyVertices(float *out);

    void cpyUv(float *out);

    void cpyIndices(unsigned int smi, int *out);

protected:
    void free();

    void allocAddContour(Contour* contour);
    void allocAddPolygon(Polygon *poly);

    void allocSetVerticesData(Polygon *poly);
    void allocSetVerticesData(Contour * contour);
    void allocSetTrianglesData(Polygon *poly);
};


}

#endif // MCASH_H
