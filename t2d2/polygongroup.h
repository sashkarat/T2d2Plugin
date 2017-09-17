#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H

#include <fstream>

#include "t2d2.h"


namespace t2d2 {

enum MCashContentOptions;
enum MCashStageOptions;

class MCash;
class MeshCash;
class Polygon;
class Borders;

class PolygonGroup
{
    Polygon *m_polygon;
    Borders *m_borders;

    float m_colliderArea;
    float m_colliderComX;
    float m_colliderComY;

    int m_colliderPathNum;

//    class ColliderCash {
//    protected:
//        class ColliderData {
//        public:
//            float *m_points;
//            size_t m_len;

//            ColliderData() : m_points(nullptr), m_len(0) {}

//            ~ColliderData() {if (m_points) delete [] m_points;}

//            void alloc(size_t len) {
//                m_len = len;
//                if (m_points) delete [] m_points;
//                m_points = new float [m_len * 2];
//            }

//            void copy(float *out) {
//                memcpy(static_cast<void*>(out),
//                       static_cast<void*>(m_points),
//                       m_len*2*sizeof(float));
//            }
//        };
//        ColliderData *m_data;
//    public:
//        size_t  m_colliderNum;

//        ColliderCash() : m_data(nullptr), m_colliderNum(0) {}

//        ~ColliderCash() {if (m_data) delete [] m_data;}

//        void alloc(size_t colliderNum) {
//            m_colliderNum = colliderNum;
//            if (m_data) delete [] m_data;
//            m_data = new ColliderData[m_colliderNum]; }

//        size_t len(size_t index) {
//            return m_data[index].m_len;
//        }

//        void copy(size_t index, float *out) {
//            m_data[index].copy(out);
//        }
//    };


public :

    PolygonGroup();
    ~PolygonGroup();

    Polygon*    polygon() {return m_polygon;}
    Polygon*    addPolygon();
    Polygon*    addPolygon(t2d2::Polygon *poly);
    void addPolygons(std::vector<t2d2::Polygon *> &polyVec);
    void        deletePolygon(Polygon *p);
    Borders*    borders() {return m_borders;}

    MeshCash *createMeshCash(int subMeshNum);
    void deleteMeshCash (MeshCash *mcash);



    void updateColliderGeometricValues();

    inline float getColliderArea() const {return m_colliderArea;}
    inline void  getColliderCOM(float *x, float *y) const {*x = m_colliderComX; *y = m_colliderComY;}
    inline float getColliderComX() const {return m_colliderComX;}
    inline float getColliderComY() const {return m_colliderComY;}
    inline int   getColliderPathNum() const {return m_colliderPathNum;}


    static void             saveToFile(PolygonGroup *pg, std::ofstream &fs);

    static PolygonGroup*    loadFromFile(std::ifstream &fs);


    bool clipBy(t2d2::PolygonGroup *clipperPg, float *trMtx);

protected:

    void deletePolygons();


    bool clipBy(t2d2::Polygon *clipperPoly);
};

}

#endif // POLYGONGROUP_H
