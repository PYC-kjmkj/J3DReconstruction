# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = QT3DReconstruction
DESTDIR = ../Release
CONFIG += release
DEFINES += _UNICODE _ENABLE_EXTENDED_ALIGNED_STORAGE WIN64
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += release
UI_DIR += .
RCC_DIR += .
include(QT3DReconstruction.pri)

HEADERS += \
    Dialog_ReconstructMesh.h \
    j3dviewer.h

SOURCES += \
    Dialog_ReconstructMesh.cpp \
    j3dviewer.cpp

FORMS += \
    dialog_reconstructmesh.ui
