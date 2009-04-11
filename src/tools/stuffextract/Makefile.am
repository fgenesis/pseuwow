## Process this file with automake to produce Makefile.in
SUBDIRS = StormLib
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include -Wall
## Build pseuwow
bin_PROGRAMS = stuffextract
stuffextract_SOURCES = 	dbcfile.cpp  Locale.cpp  MPQFile.cpp  MPQHelper.cpp  StuffExtract.cpp

stuffextract_LDADD = StormLib/libstormlib.a  ../../shared/Auth/libauth.a ../../shared/libshared.a -lbz2
