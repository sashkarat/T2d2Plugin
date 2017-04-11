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

extern "C" T2D2_EXPORT int t2d2_getAnswer();

extern "C" T2D2_EXPORT void t2d2_setLogCallback(LogCallback lcb);
extern "C" T2D2_EXPORT void t2d2_echoLog(const char *szStr);

extern "C" T2D2_EXPORT void *t2d2_polygonGroupCreate        ();
extern "C" T2D2_EXPORT void  t2d2_polygonGroupDelete        (void *pg);
extern "C" T2D2_EXPORT void *t2d2_polygonGroupGetPolygon    (void *pg);
extern "C" T2D2_EXPORT void *t2d2_polygonGetFirst           (void *p);
extern "C" T2D2_EXPORT void *t2d2_polygonGetNext            (void *p);
extern "C" T2D2_EXPORT void *t2d2_polygonGetPrev            (void *p);
extern "C" T2D2_EXPORT void  t2d2_polygonDelete             (void *p);
extern "C" T2D2_EXPORT void *t2d2_polygonAddNew             (void *p);
extern "C" T2D2_EXPORT void *t2d2_polygonGetContour         (void *p);
extern "C" T2D2_EXPORT int   t2d2_polygonGetHolesCount      (void *p);
extern "C" T2D2_EXPORT void *t2d2_polygonGetHole            (void *p, int index);
extern "C" T2D2_EXPORT void *t2d2_polygonAddHole            (void *p);
extern "C" T2D2_EXPORT void  t2d2_polygonDeleteHole         (void *p, int index);
extern "C" T2D2_EXPORT void  t2d2_polygonUpdateBBox         (void *p);
extern "C" T2D2_EXPORT void  t2d2_polygonGetBBox            (void *p, float *pXMin, float *pXMax, float *pYMin, float *pYMax);
extern "C" T2D2_EXPORT int   t2d2_pointsGetCount            (void *p);
extern "C" T2D2_EXPORT void  t2d2_pointsResize              (void *p, int size);
extern "C" T2D2_EXPORT void  t2d2_pointsGetValueVector2     (void *p, int startIndex, int count, float *out);
extern "C" T2D2_EXPORT void  t2d2_pointsSetValueVector2     (void *p, int startIndex, float *in, int count);
extern "C" T2D2_EXPORT void  t2d2_pointsRemove              (void *p, int startIndex, int count);
extern "C" T2D2_EXPORT void  t2d2_pointsAddValueVector2     (void *p, float *in, int count);

#endif // T2DPLUGIN_H
