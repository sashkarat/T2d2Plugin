#ifndef T2D2PLUGIN_H
#define T2D2PLUGIN_H

#ifdef T2D2_ANDROID_SPEC
#define T2D2_EXPORT
#else

#ifdef T2D2_WIN_SPEC


#define T2D2_EXPORT __declspec(dllexport)

#ifndef T2D2PLUGIN_LIBRARY
#define T2D2PLUGIN_IMPORT __declspec(dllimport)
#endif


#endif

#endif




#endif // T2DPLUGIN_H
