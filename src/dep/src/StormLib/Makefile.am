## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/DefScript -I$(top_builddir)/src/Client/World -I$(top_builddir)/src/Client/Realm  -Wall -D__SYS_ZLIB
AM_CFLAGS = -fPIC
## Build pseuwow
noinst_LIBRARIES = libstormlib.a
libstormlib_a_SOURCES =huffman/huff.cpp\
					wave/wave.cpp\
					SAttrFile.cpp\
                    SCommon.cpp\
                    SCompression.cpp\
                    SFileCompactArchive.cpp\
                    SFileCreateArchiveEx.cpp\
                    SFileExtractFile.cpp\
                    SFileFindFile.cpp\
                    SFileOpenArchive.cpp\
                    SFileOpenFileEx.cpp\
					SFileReadFile.cpp\
                    SListFile.cpp\
                    StormPortLinux.cpp\
                    pklib/crc32_pk.c\
                    pklib/explode.c\
                    pklib/implode.c\
                    misc/crc32.cpp\
                    misc/md5.cpp
