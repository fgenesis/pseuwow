## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include -I$(top_builddir)/src/dep/src -Wall
## Build pseuwow
bin_PROGRAMS = stuffextract
stuffextract_SOURCES = 	StuffExtract.cpp

stuffextract_LDADD =  ../../shared/Auth/libauth.a ../../shared/libshared.a  ../../dep/src/StormLib/libstormlib.a -lbz2
