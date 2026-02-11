# Makefile.am for midimet_lv2

if LIBDIR_IS_HOME
plugindir = @libdir@/.lv2
else
plugindir = @libdir@/lv2
endif

midimetplugindir = $(plugindir)/midimet.lv2

midimetplugin_LTLIBRARIES = midimet.la

midimet_la_SOURCES = \
	midievent.h \
	midimet.cpp midimet.h \
	midimet_lv2.cpp midimet_lv2.h

midimet_la_LDFLAGS = -module -avoid-version -Wl,--as-needed 
midimet_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(AM_CXXFLAGS)

# misc files which are distributed but not installed
EXTRA_DIST = \
	CMakeLists.txt cmake_config.h.in


# all generated files to be removed by "make clean"
# CLEANFILES =

