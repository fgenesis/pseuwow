## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/Client -I$(top_builddir)/shared -I$(top_builddir)/Client/DefScript -I$(top_builddir)/Client/World -I$(top_builddir)/Client/Realm  -Wall
## Build pseuwow
noinst_LIBRARIES = librealm.a
librealm_a_SOURCES =RealmSession.cpp  RealmSession.h  RealmSocket.cpp  RealmSocket.h
librealm_a_LIBADD = $(top_builddir)/shared/libshared.a $(top_builddir)/shared/Auth/libauth.a  $(top_builddir)/shared/Network/libnetwork.a


