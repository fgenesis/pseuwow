## Process this file with automake to produce Makefile.in
SUBDIRS = DefScript Realm World GUI
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include -Wall
## Build pseuwow
bin_PROGRAMS = pseuwow
pseuwow_SOURCES = 	Cli.cpp\
			DefScriptInterface.cpp\
			MemoryDataHolder.cpp\
			RemoteController.cpp\
			ControlSocket.cpp\
			main.cpp\
			PseuWoW.cpp\
			SCPDatabase.cpp\
			Realm/RealmSession.cpp\
			Realm/RealmSocket.cpp\
			World/WorldSocket.cpp\
			World/UpdateData.cpp\
			World/MapMgr.cpp
			

pseuwow_LDADD = ../shared/libshared.a\
				../shared/Auth/libauth.a\
				../shared/Network/libnetwork.a\
				DefScript/libdefscript.a\
				World/libworld.a \
				GUI/libgui.a\
				$(prefix)/libIrrKlang.so\
				../dep/lib/linux-gcc/libIrrlicht.a\
				../dep/src/zlib/libzlib.a\
				../dep/src/zthread/libZThread.a

pseuwow_LDFLAGS =-pthread 