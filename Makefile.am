SUBDIRS = src

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
	HACKING					\
	autogen.sh				\
	plib.pc.in				\
	plib-uninstalled.pc.in			\
	Doxyfile.in				\
	CMakeLists.txt				\
	cmake/CMakeLists.txt			\
	cmake/plib-config-version.cmake.in	\
	cmake/plib-config.cmake.in		\
	cmake/FindPLib.cmake			\
	cmake/PlatformDetect.cmake		\
	platforms/qnx-x86-qcc/config.h		\
	platforms/qnx-x86-qcc/plibconfig.h	\
	platforms/qnx-x86-qcc/platform.cmake	\
	platforms/win32-msvc/config.h		\
	platforms/win32-msvc/plibconfig.h	\
	platforms/win32-msvc/platform.cmake	\
	platforms/win64-msvc/config.h		\
	platforms/win64-msvc/plibconfig.h	\
	platforms/win64-msvc/platform.cmake
	

BUILT_EXTRA_DIST = \
	ChangeLog

doxygen-docs:
if DOXYGEN_DOCS_ENABLED
	@doxygen_prog@ ./Doxyfile
endif

ChangeLog:
	$(AM_V_GEN) if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./build-aux/missing --run git log --stat) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

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

DISTCLEANFILES = plibconfig.h stamp-pc-h Doxyfile

distclean-local:
	cd $(top_builddir) && rm -rf doc
	if test $(srcdir) = .; then :; else \
	    rm -f $(BUILT_EXTRA_DIST); \
	fi

.PHONY: snapshot ChangeLog

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

snapcheck:
	$(MAKE) distcheck distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

dist-hook: $(BUILT_EXTRA_DIST)
	files='$(BUILT_EXTRA_DIST)'; \
	for f in $$files; do \
	  if test -f $$f; then d=.; else d=$(srcdir); fi; \
	  rm -f $(distdir)/$$f && cp $$d/$$f $(distdir) || exit 1; done

