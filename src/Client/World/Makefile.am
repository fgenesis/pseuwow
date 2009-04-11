## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include  -Wall
## Build pseuwow
noinst_LIBRARIES = libworld.a
libworld_a_SOURCES =Bag.cpp              DynamicObject.cpp  MovementMgr.cpp  Player.cpp       World.cpp\
Bag.h                DynamicObject.h    MovementMgr.h    Player.h         World.h\
CacheHandler.cpp     GameObject.cpp     Object.cpp       SharedDefines.h  WorldPacket.cpp\
CacheHandler.h       GameObject.h       ObjectDefines.h  Unit.cpp         WorldPacket.h\
Channel.cpp          Item.cpp           Object.h         Unit.h           WorldSession.cpp\
Channel.h            Item.h             ObjMgr.cpp       UpdateData.cpp   WorldSession.h\
CMSGConstructor.cpp  ObjMgr.h         UpdateData.h     WorldSocket.cpp\
Corpse.cpp           MapMgr.cpp         Opcodes.cpp      UpdateFields.h   WorldSocket.h\
Corpse.h             MapMgr.h           Opcodes.h        UpdateMask.h

libworld_a_LIBADD = ../../shared/libshared.a ../../shared/Auth/libauth.a  ../../shared/Network/libnetwork.a
libworld_a_LIBFLAGS = -pthread


