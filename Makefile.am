# Makefile.am for midimet_lv2
#
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

EXTRA_DIST = \
	CMakeLists.txt \
	midimet.lv2/CMakeLists.txt

if LIBDIR_IS_HOME
midimet_lv2datadir=@libdir@/.lv2/midimet.lv2
else
midimet_lv2datadir=@libdir@/lv2/midimet.lv2
endif

dist_midimet_lv2data_DATA = midimet.lv2/manifest.ttl \
	midimet.lv2/midimet.ttl 


# install LV2 .so only, not .la

install-exec-hook:
	rm -f $(DESTDIR)$(midimet_lv2datadir)/*.la
	
uninstall-hook:
	rm -f $(DESTDIR)$(midimet_lv2datadir)/*.so
	rmdir $(DESTDIR)$(midimet_lv2datadir)
