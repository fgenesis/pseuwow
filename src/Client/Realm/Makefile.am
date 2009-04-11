## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include  -Wall
## Build pseuwow
noinst_LIBRARIES = librealm.a
librealm_a_SOURCES =RealmSession.cpp  RealmSession.h  RealmSocket.cpp  RealmSocket.h
librealm_a_LIBADD = $(top_builddir)/src/shared/libshared.a $(top_builddir)/src/shared/Auth/libauth.a  $(top_builddir)/src/shared/Network/libnetwork.a


