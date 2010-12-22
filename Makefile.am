SUBDIRS = src
ACLOCAL_AMFLAGS = -I m4

configexecincludedir = $(libdir)/plib/include
configexecinclude_DATA = plibconfig.h

uninstall-local:
	rm -f $(DESTDIR)$(configexecincludedir)/plibconfig.h
	rm -rf $(DESTDIR)$(libdir)/plib
	rm -rf $(DESTDIR)$(includedir)/plib

CONFIGURE_DEPENDENCIES = m4/acplib.m4

# We should always distribute these files in order to
# build PLib on any platform
EXTRA_DIST =	\
	HACKING				\
	autogen.sh			\
	plib.pc.in			\
	plib-uninstalled.pc.in		\
	plibconfig.h.win32		\
	Doxyfile.in			\
	build-aux/win32/vs9/plib.sln	\
	build-aux/win32/vs9/plib.vcproj

doxygen-docs:
if DOXYGEN_DOCS_ENABLED
	@doxygen_prog@ ./Doxyfile
endif

BUILT_SOURCES = stamp-pc-h
plibconfig.h: stamp-pc-h
	@if test -f plibconfig.h; then :; \
	else rm -f stamp-pc-h; $(MAKE) stamp-pc-h; fi
stamp-pc-h: config.status
	cd $(top_builddir) && $(SHELL) ./config.status plibconfig.h
	echo timestamp > stamp-pc-h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = plib.pc

$(pkgconfig_DATA): config.status

DISTCLEANFILES = plibconfig.h stamp-pc-h 

distclean-local:
	cd $(top_builddir) && rm -rf doc

