#ifndef MCASH_H
#define MCASH_H

#include "t2d2.h"

#include "polygongroup.h"

namespace t2d2 {

enum MCashStageOptions {
    mcosValidate        = 0x01,
    mcosTraingulate     = 0x02,
    mcosUpdateArea      = 0x04,
    mcosAllocVertices   = 0x80,
    mcosProcessUV       = 0x10,
    mcosAllocTriangles  = 0x20
};

enum MCashContentOptions {
    mcocHoles       = 0x01,
    mcocAny         = 0x02,
    mcocMesh        = 0x04,
    mcocCollider    = 0x08
};



class Contour;
class Polygon;
class PolygonGroup;

class MCash
{
public:

protected:
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
    PolygonGroup*       m_pg;

    MCashContentOptions m_mcocOpt;
    MCashStageOptions   m_mcosOpt;

    bool                m_valid;

public:

    MCash(PolygonGroup *pg);
    ~MCash();

    void allocate(t2d2::MCashContentOptions mcocOpt,
                  t2d2::MCashStageOptions mcosOpt,
                  int stride,
                  int subMeshNum);

    inline unsigned int stride()        const   {return m_stride;}

    inline unsigned int vertexNumber()  const   {return m_vertexNumber;}

    inline unsigned int subMeshNumber() const   {return m_subMeshNumber;}

    inline bool isValid() const { return m_valid;}

    unsigned int triangleNumber(unsigned int smi);


    void cpyVertices(float *out);

    void cpyUv(float *out);

    void cpyIndices(unsigned int smi, int *out);

protected:
    void free();

    bool validate();

    void allocVertexData(Polygon *poly);
    void allocVertexData(Contour* contour);

    void allocSetVerticesData(Polygon *poly);
    void allocSetVerticesData(Contour * contour);
    void allocSetTrianglesData(Polygon *poly);



    bool contentCheck(Polygon *poly);

};


}

#endif // MCASH_H
