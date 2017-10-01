#ifndef T2D2PLUGIN_H
#define T2D2PLUGIN_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)

    #ifdef T2D2PLUGIN_LIBRARY
        #define T2D2_EXPORT __declspec(dllexport)
    #else
        #define T2D2_EXPORT __declspec(dllimport)
    #endif

#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
    #define T2D2_EXPORT
#else
    #define T2D2_EXPORT
#endif

typedef void (*LogCallback)(const char *);
typedef void* T2d2Hndl;

// test func

extern "C" T2D2_EXPORT int t2d2_getAnswer();

// common func

extern "C" T2D2_EXPORT int  t2d2_version();

extern "C" T2D2_EXPORT void t2d2_msgVersion();
extern "C" T2D2_EXPORT void t2d2_setLogCallback(LogCallback lcb);
extern "C" T2D2_EXPORT void t2d2_echoLog(const char *szStr);

#ifndef ANDROID

extern "C" T2D2_EXPORT void t2d2_setLogFile(const char *szLogFileName, bool trunc);
extern "C" T2D2_EXPORT void t2d2_saveTrMatrix(const char *szFileName, float *trMtx);
extern "C" T2D2_EXPORT void t2d2_loadTrMatrix(const char *szFileName, float *trMtx);

#endif

// polygon group func

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupCreate         ();
extern "C" T2D2_EXPORT void     t2d2_polygonGroupDelete         (T2d2Hndl pg);

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupLoadFromFile   (const char *szFileName);
extern "C" T2D2_EXPORT void     t2d2_polygonGroupSaveToFile     (T2d2Hndl pg, const char *szFileName);

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupGetPolygon     (T2d2Hndl pg);
extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupAddPolygon     (T2d2Hndl pg);
extern "C" T2D2_EXPORT void     t2d2_polygonGroupDeletePolygon  (T2d2Hndl pg, T2d2Hndl poly);

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupGetBorders     (T2d2Hndl pg);

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupCreateMeshCash   (T2d2Hndl pg, int subMeshNumber, bool validate);
extern "C" T2D2_EXPORT void     t2d2_polygonGroupDeleteMeshCash   (T2d2Hndl pg, T2d2Hndl mcash);

extern "C" T2D2_EXPORT void     t2d2_polygonGroupUpdateColliderGeomValues (T2d2Hndl pg);
extern "C" T2D2_EXPORT float    t2d2_polygonGroupGetColliderArea (T2d2Hndl pg);
extern "C" T2D2_EXPORT void     t2d2_polygonGroupGetColliderCOM  (T2d2Hndl pg, float *out);
extern "C" T2D2_EXPORT int      t2d2_polygonGroupGetColliderPathNum (T2d2Hndl pg);

extern "C" T2D2_EXPORT T2d2Hndl t2d2_polygonGroupCreateColliderCash (T2d2Hndl pg);
extern "C" T2D2_EXPORT void     t2d2_polygonGroupDeleteColliderCash (T2d2Hndl pg, T2d2Hndl ccash);

extern "C" T2D2_EXPORT void     t2d2_polygonGroupAddClippingClip(T2d2Hndl pg, T2d2Hndl clipperPg, float *trMtx);
extern "C" T2D2_EXPORT bool     t2d2_polygonGroupClip(T2d2Hndl pg);
extern "C" T2D2_EXPORT bool     t2d2_polygonGroupClipBy (T2d2Hndl pg, T2d2Hndl clipperPg, float *trMtx);

extern "C" T2D2_EXPORT bool     t2d2_polygonGroupSlicePoly  (T2d2Hndl pg, T2d2Hndl poly, int gridSize);


// polygon func

extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonGetNext             (T2d2Hndl poly);
extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonGetPrev             (T2d2Hndl poly);

extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonGetContour          (T2d2Hndl poly);
extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonGetOutline          (T2d2Hndl poly);
extern "C" T2D2_EXPORT unsigned int t2d2_polygonGetHolesCount       (T2d2Hndl poly);
extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonGetHole             (T2d2Hndl poly, unsigned int index);
extern "C" T2D2_EXPORT T2d2Hndl     t2d2_polygonAddHole             (T2d2Hndl poly);
extern "C" T2D2_EXPORT void         t2d2_polygonDeleteHole          (T2d2Hndl poly, unsigned int index);
extern "C" T2D2_EXPORT void         t2d2_polygonUpdateBBox          (T2d2Hndl poly);
extern "C" T2D2_EXPORT void         t2d2_polygonGetBBox             (T2d2Hndl poly, float *out, int stride);
extern "C" T2D2_EXPORT void         t2d2_polygonUpdateIndexator     (T2d2Hndl poly, int gridSize);
extern "C" T2D2_EXPORT void         t2d2_polygonUpdateBorderGeometry(T2d2Hndl poly);

extern "C" T2D2_EXPORT void         t2d2_polygonSetUvProjectionMatrix4x4     (T2d2Hndl poly, float *data);

extern "C" T2D2_EXPORT float        t2d2_polygonGetZValue           (T2d2Hndl poly);
extern "C" T2D2_EXPORT int          t2d2_polygonGetSubMeshIndex     (T2d2Hndl poly);

extern "C" T2D2_EXPORT void         t2d2_polygonSetZValue           (T2d2Hndl poly, float zval);
extern "C" T2D2_EXPORT void         t2d2_polygonSetSubMeshIndex     (T2d2Hndl poly, int smi);


extern "C" T2D2_EXPORT void         t2d2_polygonUpdateArea          (T2d2Hndl poly);
extern "C" T2D2_EXPORT void         t2d2_polygonUpdateCOM           (T2d2Hndl poly);

extern "C" T2D2_EXPORT float        t2d2_polygonGetArea             (T2d2Hndl poly);
extern "C" T2D2_EXPORT void         t2d2_polygonGetCOM              (T2d2Hndl poly, float *out);
extern "C" T2D2_EXPORT void         t2d2_polygonGetAveragePoint     (T2d2Hndl poly, float *out);
extern "C" T2D2_EXPORT void         t2d2_polygonGetBBoxCenterPoint  (T2d2Hndl poly, float *out);

extern "C" T2D2_EXPORT void         t2d2_polygonSetPivot            (T2d2Hndl poly, float *in);
extern "C" T2D2_EXPORT void         t2d2_polygonGetPivot            (T2d2Hndl poly, float *out);

extern "C" T2D2_EXPORT void         t2d2_polygonGetFlags            (T2d2Hndl poly, bool *flags);
extern "C" T2D2_EXPORT void         t2d2_polygonSetFlags            (T2d2Hndl poly, bool *flags);


// contours func

extern "C" T2D2_EXPORT unsigned int  t2d2_contourGetLength  (T2d2Hndl cntr);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourGetValue   (T2d2Hndl cntr,
                                                             unsigned int startIndex, unsigned int length,
                                                             float* out, unsigned int stride, bool fillByZValue);

extern "C" T2D2_EXPORT unsigned int t2d2_contourGetValue2d (T2d2Hndl cntr, unsigned int startIndex, unsigned int length,
                                                             float* out);

extern "C" T2D2_EXPORT unsigned int t2d2_contourGetValue3d (T2d2Hndl cntr, unsigned int startIndex, unsigned int length,
                                                             float* out);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourSetValue   (T2d2Hndl cntr, unsigned int startIndex,
                                                             float *in, unsigned int length, int stride);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourSetValue2d (T2d2Hndl cntr, unsigned int startIndex,
                                                             float *in, unsigned int length);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourSetValue3d (T2d2Hndl cntr, unsigned int startIndex,
                                                             float *in, unsigned int length);

extern "C" T2D2_EXPORT bool          t2d2_contourRemove     (T2d2Hndl cntr, int startIndex, int count);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourAddValue   (T2d2Hndl cntr, float *in, unsigned int length, unsigned int stride);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourAddValue2d (T2d2Hndl cntr, float *in, unsigned int length);

extern "C" T2D2_EXPORT unsigned int  t2d2_contourAddValue3d (T2d2Hndl cntr, float *in, unsigned int length);

extern "C" T2D2_EXPORT void          t2d2_contourSetBorderFlags (T2d2Hndl cntr, int startIndex, int *flags, int length);

extern "C" T2D2_EXPORT void          t2d2_contourGetBorderFlags (T2d2Hndl cntr, int startIndex, int length, int * out);

extern "C" T2D2_EXPORT void          t2d2_contourUpdateBorderGeometry  (T2d2Hndl cntr);

extern "C" T2D2_EXPORT void          t2d2_contourGetNormals     (T2d2Hndl cntr, int startIndex, int length, float *out);

extern "C" T2D2_EXPORT void          t2d2_contourGetMiters      (T2d2Hndl cntr, int startIndex, int length, float *out);

extern "C" T2D2_EXPORT void          t2d2_contourGetDotPrValues (T2d2Hndl cntr, int startIndex, int length, float *out);

extern "C" T2D2_EXPORT void          t2d2_contourUpdateArea     (T2d2Hndl cntr);

extern "C" T2D2_EXPORT void          t2d2_contourUpdateCOM      (T2d2Hndl cntr);

extern "C" T2D2_EXPORT float         t2d2_contourGetArea        (T2d2Hndl cntr);

extern "C" T2D2_EXPORT void          t2d2_contourGetCOM         (T2d2Hndl cntr, float *out);

// borders params func

extern "C" T2D2_EXPORT  unsigned int    t2d2_bordersGetMask     (T2d2Hndl b);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetMask     (T2d2Hndl b, unsigned int mask);
extern "C" T2D2_EXPORT  bool            t2d2_bordersIsEnabled   (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  float           t2d2_bordersGetOffset   (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  float           t2d2_bordersGetWidth    (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  float           t2d2_bordersGetZOffset  (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  void            t2d2_bordersGetUVOffset (T2d2Hndl b, unsigned int index, float *out);
extern "C" T2D2_EXPORT  void            t2d2_bordersGetUVScale  (T2d2Hndl b, unsigned int index, float *out);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetOffset   (T2d2Hndl b, unsigned int index, float offset);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetWidth    (T2d2Hndl b, unsigned int index, float width);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetZOffset  (T2d2Hndl b, unsigned int index, float zOffset);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetUVOffset (T2d2Hndl b, unsigned int index, float* offset);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetUVScale  (T2d2Hndl b, unsigned int index, float* scale);
extern "C" T2D2_EXPORT  int             t2d2_bordersGetSubMeshIndex (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetSubMeshIndex (T2d2Hndl b, unsigned int index, int smi);
extern "C" T2D2_EXPORT  int             t2d2_bordersGetTriOrder (T2d2Hndl b, unsigned int index);
extern "C" T2D2_EXPORT  void            t2d2_bordersSetTriOrder (T2d2Hndl b, unsigned int index, int to);

// mesh cash func

extern "C" T2D2_EXPORT bool         t2d2_mcashIsValid           (T2d2Hndl mcash);
extern "C" T2D2_EXPORT unsigned int t2d2_mcashVertexNumber      (T2d2Hndl mcash);
extern "C" T2D2_EXPORT unsigned int t2d2_mcashSubMeshNumber     (T2d2Hndl mcash);
extern "C" T2D2_EXPORT unsigned int t2d2_mcashTriangleNumber    (T2d2Hndl mcash, unsigned int smi);
extern "C" T2D2_EXPORT void         t2d2_mcashGetVertices       (T2d2Hndl mcash, float *out);
extern "C" T2D2_EXPORT void         t2d2_mcashGetUv             (T2d2Hndl mcash, float *out);
extern "C" T2D2_EXPORT void         t2d2_mcashGetIndices        (T2d2Hndl mcash, unsigned int smi, int *out);

// collider cash func

extern "C" T2D2_EXPORT void     t2d2_colliderCashOffset     (T2d2Hndl ccash, float offset);
extern "C" T2D2_EXPORT float    t2d2_colliderCashArea       (T2d2Hndl ccash);
extern "C" T2D2_EXPORT void     t2d2_colliderCashCOM        (T2d2Hndl ccash, float *out);
extern "C" T2D2_EXPORT int      t2d2_colliderCashPathNum    (T2d2Hndl ccash);
extern "C" T2D2_EXPORT int      t2d2_colliderCashPathLen    (T2d2Hndl ccash, int index);
extern "C" T2D2_EXPORT void     t2d2_colliderCashPath       (T2d2Hndl ccash, int index, float *out);

// common utility

extern "C" T2D2_EXPORT bool t2d2_utilAlmostEquals               (float a, float b, int maxUlps);
extern "C" T2D2_EXPORT bool t2d2_utilSegmentIntersect           (float *a, float *b, float *c, float *d);
extern "C" T2D2_EXPORT bool t2d2_utilPointToSegmentProjection   (float *a, float *b, float *c, float *proj);
extern "C" T2D2_EXPORT bool t2d2_utilPointOnContour             (float *polyPoints, int length, int stride, float *point);
extern "C" T2D2_EXPORT bool t2d2_utilContourContains            (float *polyPoints, int length, int stride, float *point);
extern "C" T2D2_EXPORT int  t2d2_utilEdgeSelfIntersection       (float *points, int length, int stride, int index);
extern "C" T2D2_EXPORT int  t2d2_utilSegmentContourIntersection (float *segment, int strideS, float *contour, int length, int strideC);
extern "C" T2D2_EXPORT int  t2d2_utilFindNearestEdge            (float *polyPoints, int length, int stride, float *point, float *out);
extern "C" T2D2_EXPORT void t2d2_utilBBox                       (float *points, int length, int stride, float *outMin, float *outMax);
extern "C" T2D2_EXPORT void t2d2_utilAveragePoint               (float *points, int length, int stride, float *outX, float *outY);


#endif // T2DPLUGIN_H
