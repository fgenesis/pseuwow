## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -I$(top_builddir)/src/Client -I$(top_builddir)/src/shared -I$(top_builddir)/src/Client/GUI -I$(top_builddir)/src/dep/include -Wall
## Build pseuwow
bin_PROGRAMS = viewer
viewer_SOURCES =    main.cpp\
                    $(top_builddir)/src/Client/MemoryDataHolder.cpp
viewer_LDADD =  ../../dep/lib/linux-gcc/libIrrlicht.a\
                ../../Client/GUI/libgui.a\
                $(top_builddir)/src/shared/libshared.a\
                ../../dep/src/zthread/libZThread.a
viewer_LDFLAGS =-pthread 