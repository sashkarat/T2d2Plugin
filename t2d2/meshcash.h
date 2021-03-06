#ifndef MESHCASH_H
#define MESHCASH_H


#include "t2d2.h"

namespace t2d2 {

class Point;
class Contour;
class Border;
class Polygon;
class PolygonGroup;

class MeshCash
{

protected:
    class SubMeshTriangles
    {
    public:
        unsigned int m_triNum;
        int *m_tri;
        unsigned int m_triDone;
        SubMeshTriangles () : m_triNum(0), m_tri(0) {}
        ~SubMeshTriangles() {if (m_tri) delete [] m_tri;}

        void allocate() {
            if (m_tri)
                delete [] m_tri;
            m_tri = new int [m_triNum * 3];
            m_triDone = 0;
        }
    };

    unsigned int        m_vertexNumber;
    float*              m_vertices;
    float*              m_uv;

    unsigned int        m_subMeshNumber;
    SubMeshTriangles*   m_smTriangles;
    PolygonGroup*       m_pg;

    bool                m_valid;

    unsigned int        m_verticesDone;

public:
    MeshCash(PolygonGroup *pg);
    ~MeshCash();

    bool build(unsigned int subMeshNumber, bool doValidation);

    inline bool isValid() const { return m_valid;}

    inline unsigned int vertexNumber()  const   {return m_vertexNumber;}

    inline unsigned int subMeshNumber() const   {return m_subMeshNumber;}

    unsigned int triangleNumber(unsigned int smi) const;

    void cpyVertices(float *out);

    void cpyUv(float *out);

    void cpyIndices(unsigned int smi, int *out);


protected:
    void free();

    bool validate();

    void allocate();

    void set();

    bool check (Polygon * poly);

    void allocVertices (Polygon * poly);

    void allocTriangles (Polygon * poly);

    void allocContour (Contour * contour);

    void setVertices (Polygon * poly);

    void setVertices (Contour * contour);

    void setTriangles (Polygon * poly);
};


}
#endif // MESHCASH_H
