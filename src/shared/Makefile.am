## Process this file with automake to produce Makefile.in
SUBDIRS = Auth Network
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include  -Wall
## Build pseuwow
noinst_LIBRARIES = libshared.a
libshared_a_SOURCES = 	ADTFile.cpp       common.h      log.h        MapTile.h        tools.cpp    Widen.h\
ADTFile.h         DebugStuff.h  ProgressBar.cpp  tools.h      ZCompressor.cpp\
ADTFileStructs.h  libshared.a   ProgressBar.h    WDTFile.cpp  ZCompressor.h\
ByteBuffer.h      log.cpp       MapTile.cpp  SysDefs.h        WDTFile.h

