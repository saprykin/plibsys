AM_CPPFLAGS = \
	@PUTHREAD_COMPILE_IMPL_DEFINES@ \
	-DPLIB_COMPILATION

lib_LTLIBRARIES = libplib.la

libplib_la_SOURCES =	\
	pcryptohash.c	\
	pfile.c		\
	pgost3411.c	\
	phashtable.c	\
	plist.c		\
	pmain.c		\
	pmem.c		\
	pmd5.c		\
	pprocess.c	\
	psha1.c		\
	psocket.c	\
	psocketaddress.c

plibincludedir=$(includedir)/plib
plibinclude_HEADERS =   \
	patomic.h	\
	ptypes.h	\
	pmacros.h	\
	pcryptohash.h	\
	pfile.h		\
	phashtable.h	\
	plib.h		\
	plist.h		\
	pmain.h		\
	pmem.h		\
	pmutex.h	\
	pprocess.h	\
	pnetwork.h	\
	psemaphore.h	\
	pshmbuffer.h	\
	psocket.h	\
	psocketaddress.h\
	puthread.h	\
	pshm.h

noinst_HEADERS = 	\
	pgost3411.h	\
	pmd5.h		\
	psha1.h

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
	pshm.c		\
	pshmbuffer.c
if HAVE_SYSV_IPC
libplib_la_SOURCES +=		\
	psemaphore-sysv.c	\
	pshm-sysv.c
else
if HAVE_POSIX_IPC
libplib_la_SOURCES +=		\
	psemaphore-posix.c	\
	pshm-posix.c
endif
endif
endif

if HAVE_THREADS
libplib_la_SOURCES += \
	puthread.c
if HAVE_POSIX_THREADS
libplib_la_SOURCES +=		\
	puthread-posix.c	\
	pmutex-posix.c
else
if HAVE_SOLARIS_THREADS
libplib_la_SOURCES +=		\
	puthread-solaris.c	\
	pmutex-solaris.c
endif
endif
endif

if NEED_NETWORK_FUNCTIONS
libplib_la_SOURCES += \
	pnetwork.c
endif

libplib_la_CFLAGS = \
	$(P_UTHREAD_CFLAGS)
libplib_la_LIBADD = \
	$(P_LIBS_FOR_UTHREAD) $(P_IPC_LIBS)
libplib_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

EXTRA_DIST =			\
	pmutex-win.c		\
	psemaphore-win.c	\
	pshm-win.c		\
	puthread-win.c

