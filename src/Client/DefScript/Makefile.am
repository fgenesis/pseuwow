## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm  -Wall
## Build pseuwow
noinst_LIBRARIES = libdefscript.a
libdefscript_a_SOURCES =DefScriptBBFunctions.cpp\
			DefScriptFileFunctions.cpp\
			DefScriptListFunctions.cpp\
			DynamicEvent.cpp\
			DefScript.cpp\
			DefScriptFunctions.cpp\
			DefScriptTools.cpp\
			VarSet.cpp

libdefscript_a_LIBADD = $(top_builddir)/src/shared/libshared.a $(top_builddir)/src/shared/Auth/libauth.a  $(top_builddir)/src/shared/Network/libnetwork.a



