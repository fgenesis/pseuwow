## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include  -Wall
## Build pseuwow
noinst_LIBRARIES = libgui.a
libgui_a_SOURCES =CCursorController.cpp  DrawObject.cpp  MInput.h           Scene.h                   SImage.h\
CCursorController.h    DrawObject.h    PseuGUI.cpp        SceneLogin.cpp            TlTMesh.h\
CImageLoaderBLP.cpp    DrawObjMgr.cpp  PseuGUI.h          SceneWorld.cpp            TlTSector.h\
CImageLoaderBLP.h      DrawObjMgr.h    Scene.cpp          ShTlTerrainSceneNode.cpp\
CM2MeshFileLoader.cpp  SceneData.h        ShTlTerrainSceneNode.h\
CM2MeshFileLoader.h    MCamera.h       SceneGuiStart.cpp  SImage.cpp       SceneCharselection.cpp\
CIrrKlangAudioStreamLoaderMP3.cpp	CIrrKlangAudioStreamLoaderMP3.h	CIrrKlangAudioStreamMP3.cpp	CIrrKlangAudioStreamMP3.h\
ikpMP3.cpp decoder/bits.c  decoder/internal.h  decoder/mpaudec.c  decoder/mpaudec.h  decoder/mpaudectab.h  decoder/mpegaudio.h\
irrKlangSceneNode.cpp irrKlangSceneNode.h CBoneSceneNode.cpp CBoneSceneNode.h SSkinnedMesh.cpp SSkinnedMesh.h\
CMDHMemoryReadFile.cpp CMDHMemoryReadFile.h MemoryInterface.cpp MemoryInterface.h\
CWMOMeshFileLoader.cpp CWMOMeshFileLoader.h


libgui_a_LIBADD = $(top_builddir)/src/shared/libshared.a $(top_builddir)/src/shared/Auth/libauth.a  $(top_builddir)/src/shared/Network/libnetwork.a




