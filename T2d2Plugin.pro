#-------------------------------------------------
#
# Project created by QtCreator 2017-04-08T23:52:07
#
#-------------------------------------------------

CONFIG -= Qt

TARGET = t2d2plugin
TEMPLATE = lib

DEFINES += T2D2PLUGIN_LIBRARY


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clipperlib_f/clipper.cpp \
    poly2tri_f/sweep/advancing_front.cc \
    poly2tri_f/sweep/cdt.cc \
    poly2tri_f/sweep/sweep.cc \
    poly2tri_f/sweep/sweep_context.cc \
    poly2tri_f/common/shapes.cc \
    t2d2/log.cpp \
    t2d2/util.cpp \
    t2d2/bbox.cpp \
    t2d2/polygon.cpp \
    t2d2plugin.cpp \
    t2d2/mcash.cpp \
    t2d2/polygongroup.cpp \
    t2d2/contour.cpp \
    t2d2/uvprojection.cpp \
    t2d2/gridindexator.cpp

HEADERS += \
    clipperlib_f/clipper.hpp \
    poly2tri_f/poly2tri.h \
    poly2tri_f/sweep/advancing_front.h \
    poly2tri_f/sweep/cdt.h \
    poly2tri_f/sweep/sweep.h \
    poly2tri_f/sweep/sweep_context.h \
    poly2tri_f/common/shapes.h \
    poly2tri_f/common/utils.h \
    t2d2plugin.h \
    t2d2/log.h \
    t2d2/t2d2.h \
    t2d2/bbox.h \
    t2d2/polygon.h \
    t2d2/util.h \
    t2d2/mcash.h \
    t2d2/polygongroup.h \
    t2d2/contour.h \
    t2d2/uvprojection.h \
    t2d2/gridindexator.h

android {
    #message(ANDROID SPEC)
    DEFINES += T2D2_ANDROID_SPEC
    LIBS += -lsupc++ -lgnustl_static
}

win32 {
    #message(WIN SPEC)

    INST_SDK_PATH = "../t2d2tplug_sdk"

    DEFINES += T2D2_WIN_SPEC

    DLLDESTDIR = "c:/Libs"

    target.path += $${INST_SDK_PATH}
    HEADERS_TO_SDK.path = $${INST_SDK_PATH}
    HEADERS_TO_SDK.files += t2d2plugin.h

    DLL_TO_UNITY.path="../../../Unity/Terra2d2-dev/Assets/Plugins/x86_64"
    DLL_TO_UNITY.files=$${DLLDESTDIR}/$${TARGET}.dll

    INSTALLS += target HEADERS_TO_SDK DLL_TO_UNITY
}


