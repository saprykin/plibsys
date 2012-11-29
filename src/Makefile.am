AM_CPPFLAGS = \
	@PUTHREAD_COMPILE_IMPL_DEFINES@ \
	-DPLIB_COMPILATION

lib_LTLIBRARIES = libplib.la

if OS_WIN32_AND_DLL_COMPILATION
if MS_LIB_AVAILABLE
noinst_DATA = plib.lib

install_ms_lib_cmd = $(INSTALL) plib.lib $(DESTDIR)$(libdir)
uninstall_ms_lib_cmd = -rm $(DESTDIR)$(libdir)/plib.lib
endif
endif

install-ms-lib:
	$(install_ms_lib_cmd)

uninstall-ms-lib:
	$(uninstall_ms_lib_cmd)

libplib_la_SOURCES =	\
	pcryptohash.c	\
	pfile.c		\
	pgost3411.c	\
	phashtable.c	\
	pinifile.c	\
	plist.c		\
	pmain.c		\
	pmem.c		\
	pmd5.c		\
	pprocess.c	\
	psha1.c		\
	pshmbuffer.c	\
	psocket.c	\
	psocketaddress.c\
	pstring.c

plibincludedir=$(includedir)/plib
plibinclude_HEADERS =   \
	patomic.h	\
	ptypes.h	\
	pmacros.h	\
	pcondvariable.h	\
	pcryptohash.h	\
	pfile.h		\
	pgost3411.h	\
	phashtable.h	\
	pinifile.h	\
	plib.h		\
	plist.h		\
	pmain.h		\
	pmd5.h		\
	pmem.h		\
	pmutex.h	\
	pprocess.h	\
	psemaphore.h	\
	psha1.h		\
	pshm.h		\
	pshmbuffer.h	\
	psocket.h	\
	psocketaddress.h\
	pstring.h	\
	puthread.h

if HAVE_GCC_BUILTINS_FOR_ATOMIC_OPERATIONS
libplib_la_SOURCES += \
	patomic-gcc.c
else
libplib_la_SOURCES += \
	patomic.c
endif

if HAVE_IPC
libplib_la_SOURCES +=	\
	psemaphore.c	\
	pshm.c
if HAVE_SYSV_IPC
libplib_la_SOURCES +=		\
	psemaphore-sysv.c	\
	pshm-sysv.c
else
if HAVE_POSIX_IPC
libplib_la_SOURCES +=		\
	psemaphore-posix.c	\
	pshm-posix.c
else
if HAVE_WIN32_IPC
libplib_la_SOURCES +=		\
	psemaphore-win.c	\
	pshm-win.c
endif
endif
endif
else
libplib_la_SOURCES +=		\
	psemaphore-none.c	\
	pshm-none.c
endif

if HAVE_THREADS
libplib_la_SOURCES += \
	puthread.c
if HAVE_POSIX_THREADS
libplib_la_SOURCES +=		\
	puthread-posix.c	\
	pmutex-posix.c		\
	pcondvariable-posix.c
else
if HAVE_SOLARIS_THREADS
libplib_la_SOURCES +=		\
	puthread-solaris.c	\
	pmutex-solaris.c	\
	pcondvariable-solaris.c
else
if HAVE_WIN32_THREADS
libplib_la_SOURCES +=		\
	puthread-win.c	\
	pmutex-win.c	\
	pcondvariable-win.c
endif
endif
endif
else
libplib_la_SOURCES +=		\
	puthread-none.c		\
	pmutex-none.c		\
	pcondvariable-none.c
endif

install-data-local: install-ms-lib install-def-file
	@if test -f $(plibincludedir)/plist.h ; then						\
		echo "*** Old headers found in $(plibincludedir). You should remove the" ;	\
		echo "*** contents of this directory and type 'make install' again." ;		\
		false ;										\
	fi

uninstall-local: uninstall-ms-lib uninstall-def-file

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32_AND_DLL_COMPILATION
export_symbols = -export-symbols plib.def
plib_def = plib.def

install-def-file:
	$(INSTALL) plib.def $(DESTDIR)$(libdir)/plib.def

uninstall-def-file:
	-rm $(DESTDIR)$(libdir)/plib.def
else
install-def-file:
uninstall-def-file:

export_symbols = $(LIBTOOL_EXPORT_OPTIONS)
endif

libplib_la_CFLAGS = \
	$(P_UTHREAD_CFLAGS) $(PLIB_EXTRA_CFLAGS)
libplib_la_DEPENDENCIES = $(plib_def)
libplib_la_LIBADD = \
	$(P_LIBS_FOR_UTHREAD) $(P_IPC_LIBS) $(P_LIBS_EXTRA)
libplib_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) 	\
	-export-dynamic $(no_undefined) $(export_symbols)

EXTRA_DIST =			\
	CMakeLists.txt		\
	pcondvariable-win.c	\
	pmutex-win.c		\
	psemaphore-win.c	\
	pshm-win.c		\
	puthread-win.c

plib.lib: libplib.la plib.def
	lib -machine:@LIB_EXE_MACHINE_FLAG@ -name:libplib-$(LT_CURRENT_MINUS_AGE).dll -def:plib.def -out:$@

