## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/Client -I$(top_builddir)/shared -I$(top_builddir)/Client/DefScript -I$(top_builddir)/Client/World -I$(top_builddir)/Client/Realm  -Wall
## Build pseuwow
noinst_LIBRARIES = libgui.a
libgui_a_SOURCES =CCursorController.cpp  DrawObject.cpp  MInput.h           Scene.h                   SImage.h\
CCursorController.h    DrawObject.h    PseuGUI.cpp        SceneLogin.cpp            TlTMesh.h\
CImageLoaderBLP.cpp    DrawObjMgr.cpp  PseuGUI.h          SceneWorld.cpp            TlTSector.h\
CImageLoaderBLP.h      DrawObjMgr.h    Scene.cpp          ShTlTerrainSceneNode.cpp\
CM2MeshFileLoader.cpp  SceneData.h        ShTlTerrainSceneNode.h\
CM2MeshFileLoader.h    MCamera.h       SceneGuiStart.cpp  SImage.cpp

libgui_a_LIBADD = $(top_builddir)/shared/libshared.a $(top_builddir)/shared/Auth/libauth.a  $(top_builddir)/shared/Network/libnetwork.a



