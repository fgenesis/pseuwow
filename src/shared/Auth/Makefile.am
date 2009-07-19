## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm -I$(top_builddir)/src/dep/include -Wall
## Build pseuwow
noinst_LIBRARIES = libauth.a
libauth_a_SOURCES = AuthCrypt.cpp  BigNumber.cpp  md5.c  Sha1.cpp  Hmac.cpp SARC4.cpp
