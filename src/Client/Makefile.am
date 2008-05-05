## Process this file with automake to produce Makefile.in
SUBDIRS = DefScript Realm World GUI
AM_CPPFLAGS = -I$(top_builddir)/Client -I$(top_builddir)/shared -I$(top_builddir)/Client/DefScript -I$(top_builddir)/Client/World -I$(top_builddir)/Client/Realm  -Wall
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
			

pseuwow_LDADD = ../shared/libshared.a ../shared/Auth/libauth.a ../shared/Network/libnetwork.a DefScript/libdefscript.a World/libworld.a GUI/libgui.a 
pseuwow_LDFLAGS =-pthread