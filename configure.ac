AC_PREREQ(2.54)

m4_include(m4/acplib.m4)

dnl Making releases:
dnl   plib_micro_version += 1;
dnl   plib_interface_age += 1;
dnl   plib_binary_age += 1;
dnl if any functions have been added, set plib_interface_age to 0;
dnl if backwards compatibility has been broken,
dnl set plib_binary_age _AND_ plib_interface_age to 0.
m4_define([plib_major_version], [0])
m4_define([plib_minor_version], [0])
m4_define([plib_micro_version], [1])
m4_define([plib_interface_age], [0])
m4_define([plib_binary_age], [m4_eval(100 * plib_minor_version + plib_micro_version)])
m4_define([plib_version], [plib_major_version.plib_minor_version.plib_micro_version])

dnl Libtool version related macros
m4_define([plib_lt_release], [plib_major_version.plib_minor_version])
m4_define([plib_lt_current],
          [m4_eval(100 * plib_minor_version + plib_micro_version - plib_interface_age)])
m4_define([plib_lt_revision], [plib_interface_age])
m4_define([plib_lt_age], [m4_eval(plib_binary_age - plib_interface_age)])
m4_define([plib_lt_current_minus_age], [m4_eval(plib_lt_current - plib_lt_age)])

AC_INIT([plib], [plib_version], [xelfium@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

dnl Enable silent build when available (Automake 1.11)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/plib.h])
AC_CONFIG_HEADERS([config.h])

AM_MAINTAINER_MODE

PLIB_MAJOR_VERSION=plib_major_version
PLIB_MINOR_VERSION=plib_minor_version
PLIB_MICRO_VERSION=plib_micro_version
PLIB_INTERFACE_AGE=plib_interface_age
PLIB_BINARY_AGE=plib_binary_age
PLIB_VERSION=plib_version

AC_SUBST(PLIB_MAJOR_VERSION)
AC_SUBST(PLIB_MINOR_VERSION)
AC_SUBST(PLIB_MICRO_VERSION)
AC_SUBST(PLIB_VERSION)
AC_SUBST(PLIB_INTERFACE_AGE)
AC_SUBST(PLIB_BINARY_AGE)

AC_DEFINE(PLIB_MAJOR_VERSION, [plib_major_version], [Define to the PLib major version])
AC_DEFINE(PLIB_MINOR_VERSION, [plib_minor_version], [Define to the PLib minor version])
AC_DEFINE(PLIB_MICRO_VERSION, [plib_micro_version], [Define to the PLib micro version])
AC_DEFINE(PLIB_INTERFACE_AGE, [plib_interface_age], [Define to the PLib interface age])
AC_DEFINE(PLIB_BINARY_AGE, [plib_binary_age], [Define to the PLib binary age])

dnl Libtool versioning
LT_RELEASE=plib_lt_release
LT_CURRENT=plib_lt_current
LT_REVISION=plib_lt_revision
LT_AGE=plib_lt_age
LT_CURRENT_MINUS_AGE=plib_lt_current_minus_age
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)

AC_CANONICAL_HOST

AC_MSG_CHECKING([for platform])
case $host in
	*-*-linux*)
		plib_os_native=linux
		;;
	*-*-qnx*)
		plib_os_native=qnx
		;;
	*-*-solaris*)
		plib_os_native=solaris
		;;
	*-*-mingw*)
		plib_os_native=mingw
		;;
	*-*-darwin*)
		plib_os_native=macosx
		;;
esac
AC_MSG_RESULT([$plib_os_native])

AC_MSG_CHECKING([for Win32])
LIB_EXE_MACHINE_FLAG=X86
case "$host" in
*-*-mingw*)
	case "$host" in
	x86_64-*-*)
		LIB_EXE_MACHINE_FLAG=X64
		;;
	esac
    	;;
*)
	;;
esac

if test x"$plib_os_native" = "xmingw"; then
	AC_MSG_RESULT([$LIB_EXE_MACHINE_FLAG])
else
	AC_MSG_RESULT([no])
fi
AC_SUBST(LIB_EXE_MACHINE_FLAG)


if test x"$plib_os_native" = "xmingw"; then
  AC_CHECK_TOOL(NM, nm, no)
  if test "$NM" = no; then
    AC_MSG_ERROR([*** Could not find an implementation of nm in your PATH.])
  fi
  AC_CHECK_TOOL(RANLIB, ranlib, :)
  AC_CHECK_PROG(ms_librarian, [lib.exe], [yes], [no])
fi
AM_CONDITIONAL(MS_LIB_AVAILABLE, [test x$ms_librarian = xyes])

dnl Configuration arguments
AC_ARG_ENABLE(threads,
	      [AC_HELP_STRING([--enable-threads],
			      [turn on basic thread support [default=yes]
			       ([=no] will override --with-threads)])],,
	      [enable_threads=yes])

AC_ARG_ENABLE(ipc,
	      [AC_HELP_STRING([--enable-ipc],
			      [turn on basic IPC support [default=yes]
			       ([=no] will override --with-ipc)])],,
	      [enable_ipc=yes])

AC_ARG_ENABLE(doxygen-docs,
	      [AC_HELP_STRING([--enable-doxygen-docs],
			     [build DOXYGEN documentation (requires Doxygen) [default=auto]])],
	      [enable_doxygen_docs=$enableval],
	      [enable_doxygen_docs=auto])

if test "x$enable_threads" != "xyes"; then
	enable_threads=no
fi

if test "x$enable_ipc" != "xyes"; then
	enable_ipc=no
fi

if [test "x$enable_doxygen_docs" != "xyes"] || [test "x$enable_doxygen_docs" != "xauto"]; then
	enable_doxygen_doc=no
fi

dnl Check programs and stuff
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

AM_DISABLE_STATIC
AM_PROG_AR
AM_PROG_LIBTOOL

if test x"$plib_os_native" = "xmingw"; then
	if test x$enable_static = xyes -a x$enable_shared = xyes; then
		AC_MSG_ERROR([Can not build both shared and static at the same time on Windows.])
	fi
	if test x$enable_static = xyes; then
		plib_win32_static_compilation=yes
		PLIB_WIN32_STATIC_COMPILATION_DEFINE="#define PLIB_STATIC_COMPILATION 1"
    		AC_SUBST(PLIB_WIN32_STATIC_COMPILATION_DEFINE)
  	fi
fi
AM_CONDITIONAL(OS_WIN32_AND_DLL_COMPILATION, [test x$plib_os_native = xmingw -a x$plib_win32_static_compilation != xyes])

dnl Ensure MSVC-compatible struct packing convention is used when
dnl compiling for Win32 with gcc.
dnl What flag to depends on gcc version: gcc3 uses "-mms-bitfields", while
dnl gcc2 uses "-fnative-struct".
if test x"$plib_os_native" = xmingw; then
	if test x"$GCC" = xyes; then
		msnative_struct=''
		AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
		if test -z "$ac_cv_prog_CC"; then
			our_gcc="$CC"
		else
			our_gcc="$ac_cv_prog_CC"
		fi
		case `$our_gcc --version | sed -e 's,\..*,.,' -e q` in
		2.)
			if $our_gcc -v --help 2>/dev/null | grep fnative-struct >/dev/null; then
				msnative_struct='-fnative-struct'
			fi
		;;
		*)
			if $our_gcc -v --help 2>/dev/null | grep ms-bitfields >/dev/null; then
				msnative_struct='-mms-bitfields'
			fi
		;;
		esac
		if test x"$msnative_struct" = x ; then
			AC_MSG_RESULT([no way])
			AC_MSG_WARN([produced libraries might be incompatible with MSVC-compiled code])
		else
			CFLAGS="$CFLAGS $msnative_struct"
			AC_MSG_RESULT([${msnative_struct}])
		fi
	fi
fi
PLIB_EXTRA_CFLAGS="${msnative_struct}"
AC_SUBST(PLIB_EXTRA_CFLAGS)

dnl Check header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h fcntl.h sys/socket.h netinet/in.h signal.h errno.h arpa/inet.h sched.h sys/poll.h])
AC_CHECK_HEADERS([float.h limits.h values.h])

dnl Check for types
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(__int64)

if test x$ac_cv_sizeof_long = x8 || test x$ac_cv_sizeof_long_long = x8 || test x$ac_cv_sizeof___int64 = x8; then
	:
else
	AC_MSG_ERROR([
		*** PLib requires a 64 bit type. You might want to consider
		*** using the GNU C compiler.
	])
fi

if test x$ac_cv_sizeof_long_long = x8; then
	# long long is a 64 bit integer.
	AC_MSG_CHECKING(for format to printf and scanf a puint64)
	AC_CACHE_VAL(plib_cv_long_long_format,[
		for format in ll q I64; do
		  AC_TRY_RUN([#include <stdio.h>  
			int main()
			{
				long long b, a = -0x3AFAFAFAFAFAFAFALL;
				char buffer[1000];
				sprintf (buffer, "%${format}u", a);
  				sscanf (buffer, "%${format}u", &b);
				exit (b!=a);
			}
			],
			[plib_cv_long_long_format=${format}
			break],
			[],[:])
		done])
	if test -n "$plib_cv_long_long_format"; then
		AC_MSG_RESULT(%${plib_cv_long_long_format}u)
		AC_DEFINE(HAVE_LONG_LONG_FORMAT, 1, [define if system printf can print long long])
		if test x"$plib_cv_long_long_format" = xI64; then
		    AC_DEFINE(HAVE_INT64_AND_I64, 1, [define to support printing 64-bit integers with format I64])
		fi
        else
		AC_MSG_RESULT(none)
        fi
elif test x$ac_cv_sizeof___int64 = x8; then
	# __int64 is a 64 bit integer.
	AC_MSG_CHECKING(for format to printf and scanf a puint64)
	# We know this is MSVCRT.DLL, and what the formats are
	plib_cv_long_long_format=I64
	AC_MSG_RESULT(%${plib_cv_long_long_format}u)
        AC_DEFINE(HAVE_LONG_LONG_FORMAT, 1, [define if system printf can print long long])
	AC_DEFINE(HAVE_INT64_AND_I64, 1, [define to support printing 64-bit integers with format I64])
fi

AC_CHECK_TYPES([socklen_t], have_socklen_t=yes, have_socklen_t=no, [#include <sys/socket.h>])
AC_CHECK_TYPES([struct sockaddr_storage], have_sockaddr_storage=yes, have_sockaddr_storage=no, [#include <sys/socket.h>])

if test "x$plib_os_native" = xmingw; then
	have_sockaddr_storage=yes
fi

if test "x$have_socklen_t" = xno; then
	AC_MSG_CHECKING(for appropriate definition for socklen_t)
	PLIB_CHECK_COMPILE_WARNINGS([#include <sys/types.h>
				     #include <sys/socket.h>
				     #include <stdlib.h>
				     int main ()
				     {
					int len;
					recvfrom (0, NULL, 0, 0, NULL, &len);
					return 0;
				     }],
				    plib_socklen_type=int,
				    [PLIB_CHECK_COMPILE_WARNINGS([#include <sys/types.h>
				     				  #include <sys/socket.h>
								  #include <stdlib.h>
				     				  int main ()
				     				  {
									size_t len;
									recvfrom (0, NULL, 0, 0, NULL, &len);
									return 0;
				     				  }],
								 plib_socklen_type=size_t)])
	AC_MSG_RESULT($plib_socklen_type)
fi

dnl Check for some functions
AC_CHECK_FUNCS([memmove nanosleep poll])

dnl Check for bytesex stuff
AC_C_BIGENDIAN
if test x$ac_cv_c_bigendian = xuniversal; then
	AC_TRY_COMPILE([#include <endian.h>],
		       [#if __BYTE_ORDER == __BIG_ENDIAN
			#else
			#error Not a big endian. 
			#endif],
		       ac_cv_c_bigendian=yes,
		       AC_TRY_COMPILE([#include <endian.h>],
				      [#if __BYTE_ORDER == __LITTLE_ENDIAN
				       #else
				       #error Not a little endian.
				       #endif],
				      ac_cv_c_bigendian=no,
				      AC_MSG_WARN([Could not determine endianness.])))
fi

dnl Check additional type sizes
AC_CHECK_SIZEOF(size_t)

dnl Try to figure out whether psize, pssize should be long or int
AC_MSG_CHECKING([for the appropriate definition for size_t])

case $ac_cv_sizeof_size_t in
	$ac_cv_sizeof_short) 
		plib_size_type=short
		;;
	$ac_cv_sizeof_int) 
		plib_size_type=int
		;;
	$ac_cv_sizeof_long) 
		plib_size_type=long
		;;
	$ac_cv_sizeof_long_long)
		plib_size_type='long long'
		;;
	$ac_cv_sizeof__int64)
		plib_size_type='__int64'
		;;
	*)  AC_MSG_ERROR([No type matching size_t in size])
	;;
esac

dnl If int/long are the same size, we see which one produces
dnl warnings when used in the location as size_t (this matters
dnl on AIX with xlc).
if test $ac_cv_sizeof_size_t = $ac_cv_sizeof_int &&
   test $ac_cv_sizeof_size_t = $ac_cv_sizeof_long; then
	PLIB_CHECK_COMPILE_WARNINGS([#if defined(_AIX) && !defined(__GNUC__)
				     #pragma options langlvl=stdc89
				     #endif
				     #include <stddef.h> 
				     int main ()
				     {
					size_t s = 1;
					unsigned int *size_int = &s;
					return (int)*size_int;
				     }],
				    plib_size_type=int,
				    [PLIB_CHECK_COMPILE_WARNINGS([#if defined(_AIX) && !defined(__GNUC__)
								  #pragma options langlvl=stdc89
								  #endif
								  #include <stddef.h>
								  int main ()
								  {
									size_t s = 1;
									unsigned long *size_long = &s;
									return (int)*size_long;
								  }],
								 plib_size_type=long)])
fi

AC_MSG_RESULT(unsigned $plib_size_type)

dnl Find pkg-config
AC_PATH_PROG(PKG_CONFIG, [pkg-config], [no])
if test "x$PKG_CONFIG" = xno; then
	AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/])
fi

if $PKG_CONFIG --atleast-pkgconfig-version 0.16; then
	:
else
	AC_MSG_ERROR([*** pkg-config too old; version 0.16 or better required.])
fi

dnl p_atomic_* tests for gcc
AC_MSG_CHECKING([whether to use assembler code for atomic operations])

plib_cv_gcc_has_builtin_atomic_operations=no
if test x"$GCC" = xyes; then
	AC_MSG_CHECKING([whether GCC supports build-in atomic intrinsics])
	AC_TRY_LINK([],
		    [int i;
	 	     __sync_synchronize ();
		     __sync_bool_compare_and_swap (&i, 0, 1);
		     __sync_fetch_and_add (&i, 1);
		    ],
		    [plib_cv_gcc_has_builtin_atomic_operations=yes],
		    [plib_cv_gcc_has_builtin_atomic_operations=no])

	AC_MSG_RESULT($plib_cv_gcc_has_builtin_atomic_operations)
	if test $plib_cv_gcc_has_builtin_atomic_operations = yes; then
		plib_memory_barrier_needed=yes
	else
		case $host_cpu in
		i386)
			AC_MSG_RESULT([none])
			plib_memory_barrier_needed=no
		;;
		i?86)
			AC_MSG_RESULT([i486])
			AC_DEFINE_UNQUOTED(P_ATOMIC_I486, 1, [i486 atomic implementation])
			plib_memory_barrier_needed=no
		;;
		sparc*)
			SPARCV9_WARNING="Try to rerun configure with CFLAGS='-mcpu=v9',
				 when you are using a sparc with v9 instruction set (most
				 sparcs nowadays). This will make the code for atomic
				 operations much faster. The resulting code will not run
				 on very old sparcs though."
			AC_LINK_IFELSE([AC_LANG_SOURCE([[
					main () {
						int tmp1, tmp2, tmp3;
						__asm__ __volatile__("casx [%2], %0, %1"
							: "=&r" (tmp1), "=&r" (tmp2) : "r" (&tmp3));
					}]])],
			AC_MSG_RESULT([sparcv9])
			AC_DEFINE_UNQUOTED(P_ATOMIC_SPARCV9, 1, [sparcv9 atomic implementation]),
			AC_MSG_RESULT([no])
			AC_MSG_WARN([[$SPARCV9_WARNING]]))
			plib_memory_barrier_needed=yes
		;;
		alpha*)
			AC_MSG_RESULT([alpha])
			AC_DEFINE_UNQUOTED(P_ATOMIC_ALPHA, 1, [alpha atomic implementation])
			plib_memory_barrier_needed=yes
		;;
		x86_64)
			AC_MSG_RESULT([x86_64])
			AC_DEFINE_UNQUOTED(P_ATOMIC_X86_64, 1, [x86_64 atomic implementation])
			plib_memory_barrier_needed=no
		;;
		powerpc*)
			AC_MSG_RESULT([powerpc])
			AC_DEFINE_UNQUOTED(P_ATOMIC_POWERPC, 1, [powerpc atomic implementation])
			plib_memory_barrier_needed=yes
			AC_MSG_CHECKING([whether asm supports numbered local labels])
			AC_TRY_COMPILE(
					,[
					__asm__ __volatile__ ("1:       nop\n"
							      "         bne-    1b")
					 ],[
					  AC_DEFINE_UNQUOTED(ASM_NUMERIC_LABELS, 1, [define if asm blocks can use numeric local labels])
					  AC_MSG_RESULT([yes])
					 ],[
					  AC_MSG_RESULT([no])
					 ])
		;;
		ia64)
			AC_MSG_RESULT([ia64])
			AC_DEFINE_UNQUOTED(P_ATOMIC_IA64, 1, [ia64 atomic implementation])
			plib_memory_barrier_needed=yes
		;;
		s390|s390x)
			AC_MSG_RESULT([s390])
			AC_DEFINE_UNQUOTED(P_ATOMIC_S390, 1, [s390 atomic implementation])
			plib_memory_barrier_needed=no
		;;
		arm*)
			AC_MSG_RESULT([arm])
			AC_DEFINE_UNQUOTED(P_ATOMIC_ARM, 1, [arm atomic implementation])
			plib_memory_barrier_needed=no
		;;
		crisv32*|etraxfs*)
			AC_MSG_RESULT([crisv32])
			AC_DEFINE_UNQUOTED(P_ATOMIC_CRISV32, 1, [crisv32 atomic implementation])
			plib_memory_barrier_needed=no
		;;
		cris*|etrax*)
			AC_MSG_RESULT([cris])
			AC_DEFINE_UNQUOTED(P_ATOMIC_CRIS, 1, [cris atomic implementation])
			plib_memory_barrier_needed=no
		;;
		*)
			AC_MSG_RESULT([none])
			plib_memory_barrier_needed=yes
		;;
		esac
	fi
else
	if test x"$plib_os_native" = xmingw; then
		dnl For Windows but not using gcc. No barriers needed then either.
    		plib_memory_barrier_needed=no
  	fi
fi

AM_CONDITIONAL(HAVE_GCC_BUILTINS_FOR_ATOMIC_OPERATIONS,
	       [test $plib_cv_gcc_has_builtin_atomic_operations = yes])

AC_MSG_CHECKING([if building for some Win32 platform])
case "$host" in
*-*-mingw*)
	platform_win32=yes
	;;
*)
	platform_win32=no
	;;
esac
AC_MSG_RESULT($platform_win32)
AM_CONDITIONAL(PLATFORM_WIN32, test x$platform_win32 = xyes)

dnl Check configure arguments
AC_ARG_WITH(threads,
	    [AC_HELP_STRING([--with-threads=@<:@none/posix/solaris/win32@:>@],
			    [specify a thread implementation to use])],
	    [if test "x$with_threads" = x; then
		want_threads=yes
	    else
		want_threads=$with_threads
	    fi],
	   [want_threads=yes])
if test "x$enable_threads" = "xno"; then
	want_threads=no
fi

AC_ARG_WITH(ipc,
	    [AC_HELP_STRING([--with-ipc=@<:@none/sysv/posix/win32@:>@],
			    [specify an IPC implementation to use])],
	    [if test "x$with_ipc" = x; then
		want_ipc=yes
	    else
		want_ipc=$with_ipc
	    fi],
	   [want_ipc=yes])
if test "x$enable_ipc" = "xno"; then
	want_ipc=no
fi

THREAD_NO_IMPLEMENTATION="You do not have any known thread system on your
		computer, PLib will not have a default thread implementation."

THREAD_LIBS_NOT_FOUND_1="Can't find the libraries for the thread implementation
		"

THREAD_LIBS_NOT_FOUND_2=". Please choose another thread implementation or
		provide information on your thread implementation.
		You can also run 'configure --disable-threads' 
		to compile without thread support."

AIX_COMPILE_INFO="AIX's C compiler needs to be called by a different name, when
		linking threaded applications. As PLib cannot do that 
		automatically, you will get an linkg error everytime you are 
		not using the right compiler. In that case you have to relink 
		with the right compiler. Ususally just '_r' is appended 
		to the compiler name."

dnl determination of thread implementation
dnl have_threads=no   means no thread support
dnl have_threads=none means no default thread implementation
if test "x$want_threads" = xnone; then
	have_threads=none
fi

if test "x$want_threads" = xposix || test "x$want_threads" = xsolaris; then
	have_threads="$want_threads";
else
	have_threads=no
fi

if test "x$want_threads" = xyes || test "x$want_threads" = xwin32; then
       	case $host in
               	*-*-mingw*)
		have_threads=win32
		want_threads=yes
		;;
	esac
fi

if test "x$want_threads" = xyes || test "x$want_threads" = xposix \
				|| test "x$want_threads" = xsolaris; then
	dnl -D_POSIX4_DRAFT_SOURCE -D_POSIX4A_DRAFT10_SOURCE is for DG/UX
	dnl -U_OSF_SOURCE is for Digital UNIX 4.0d
	PUTHREAD_COMPILE_IMPL_DEFINES="-D_POSIX4_DRAFT_SOURCE -D_POSIX4A_DRAFT10_SOURCE -U_OSF_SOURCE"
	plib_save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $PUTHREAD_COMPILE_IMPL_DEFINES"
	if test "x$have_threads" = xno; then
		AC_TRY_COMPILE([#include <pthread.h>],
				[pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;],
				have_threads=posix)
	fi
	if test "x$have_threads" = xno; then
		AC_TRY_COMPILE([#include <thread.h>
				#include <synch.h>],
				[mutex_t m = DEFAULTMUTEX;],
				have_threads=solaris)
	fi
	dnl Tru64Unix requires -pthread to find pthread.h.
	CPPFLAGS="$CPPFLAGS -pthread"
	if test "x$have_threads" = xno; then
	AC_TRY_COMPILE([#include <pthread.h>],
		       [pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;],
		       have_threads=posix)
	fi
	CPPFLAGS="$plib_save_CPPFLAGS"
fi

if test "x$want_threads" = xnone; then
	have_threads=none
fi

AC_MSG_CHECKING(for thread implementation)

if test "x$have_threads" = xno && test "x$want_threads" != xno; then
	AC_MSG_RESULT(none available)
        AC_MSG_WARN($THREAD_NO_IMPLEMENTATION)
else
	AC_MSG_RESULT($have_threads)
fi

dnl Finally disable threads support
if test "x$have_threads" = xnone; then
	have_threads=no
fi

dnl Determination of P_UTHREAD_CFLAGS

P_UTHREAD_LIBS=
P_UTHREAD_CFLAGS=

dnl Test programs for basic POSIX and Solaris threads functionality
m4_define([plib_pthread_test],
	  [#include <pthread.h> 
  	  int check_me = 0;
	  void* func(void* data) {check_me = 42; return &check_me;}
	  int main()
	  {
		pthread_t t; 
		void *ret;
		pthread_create (&t, 0, func, 0);
		pthread_join (t, &ret);
		return (check_me != 42 || ret != &check_me);
	  }
])

m4_define([plib_thread_test],
	[#include <thread.h> 
	int check_me = 0;
	void* func(void* data) {check_me = 42; return &check_me;}
	int main()
	{
		thread_t th; 
		void *ret;
		thr_create (NULL, 0, func, NULL, THR_BOUND,  &th);
		thr_wait (th, NULL, &ret);
		return (check_me != 42 || ret != &check_me);
	}
])

m4_define([plib_sched_test],
	[#include <sched.h> 
	int main()
	{ 
		sched_yield ();
		return 0;
	}
])

if test x"$have_threads" != xno; then
	if test x"$have_threads" = xposix; then
		dnl First we test for posix, whether -pthread or -pthreads do the trick as 
		dnl both CPPFLAG and LIBS. 
		dnl One of them does for most gcc versions and some other platforms/compilers
		dnl too and could be considered as the canonical way to go. 
		case $host in
			*-*-solaris*)
			dnl These compiler/linker flags work with both Sun Studio and gcc
			dnl Sun Studio expands -mt to -D_REENTRANT and -lthread
			dnl gcc expands -pthreads to -D_REENTRANT -D_PTHREADS -lpthread
			P_UTHREAD_CFLAGS="-D_REENTRANT -D_PTHREADS"
			P_UTHREAD_LIBS="-lpthread -lthread"
			;;
			*)
			for flag in "" pthread pthreads mt; do
				plib_save_CFLAGS="$CFLAGS"
				if test x"$flag" = x; then
					add_thread_flag=""
				else
					add_thread_flag="-$flag"
				fi
				CFLAGS="$plib_save_CFLAGS $add_thread_flag"
				AC_TRY_RUN(AC_LANG_SOURCE([plib_pthread_test]),
					   plib_flag_works=yes,
					   plib_flag_works=no,
					   [AC_LINK_IFELSE([AC_LANG_SOURCE([plib_pthread_test])],
							   plib_flag_works=yes,
							   plib_flag_works=no)])
				CFLAGS="$plib_save_CFLAGS"
				if test $plib_flag_works = yes; then
					if test x"$flag" != x; then
						P_UTHREAD_CFLAGS=-$flag
						P_UTHREAD_LIBS=-$flag
					fi
				break;
				fi
			done
			;;
		esac
	fi

	if test x"$have_threads" = xsolaris; then
		P_UTHREAD_CFLAGS="-D_REENTRANT -D_PTHREADS"
		P_UTHREAD_LIBS="-lpthread -lthread"
	fi

	if test x"$P_UTHREAD_CFLAGS" = x; then
		dnl The canonical -pthread[s] does not work. Try something different.
		case $host in
			*-aix*)
			if test x"$GCC" = xyes; then
				dnl GCC 3.0 and above needs -pthread. 
				dnl Should be coverd by the case above.
				dnl GCC 2.x and below needs -mthreads
				P_UTHREAD_CFLAGS="-mthreads"		
				P_UTHREAD_LIBS=$P_UTHREAD_CFLAGS
			else 
				dnl We are probably using the aix compiler. Normaly a 
				dnl program would have to be compiled with the _r variant
				dnl of the corresponding compiler, but we as PLib cannot 
				dnl do that: but the good news is that for compiling the
				dnl only difference is the added -D_THREAD_SAFE compile 
				dnl option. This is according to the "C for AIX User's 
				dnl Guide".
				P_UTHREAD_CFLAGS="-D_THREAD_SAFE"
			fi
			;;
			*-dg-dgux*)
			dnl DG/UX
			P_UTHREAD_CFLAGS="-D_REENTRANT -D_POSIX4A_DRAFT10_SOURCE"
			;;
			*-sysv5uw7*)
			dnl UnixWare 7 
			dnl We are not using gcc with -pthread. Catched above.
			P_UTHREAD_CFLAGS="-Kthread"
			P_UTHREAD_LIBS=$P_UTHREAD_CFLAGS
			;;
			*)
			P_UTHREAD_CFLAGS="-D_REENTRANT"
			;;
		esac
 
	fi

	AC_MSG_CHECKING(thread related cflags)
	AC_MSG_RESULT($P_UTHREAD_CFLAGS)
	CPPFLAGS="$CPPFLAGS $P_UTHREAD_CFLAGS"

	dnl Determination of P_UTHREAD_LIBS
	case $have_threads in
		posix|solaris)
		plib_save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $PUTHREAD_COMPILE_IMPL_DEFINES"
		if test x"$P_UTHREAD_LIBS" = x; then
			case $host in
				*-aix*)
				dnl We are not using gcc (would have set P_UTHREAD_LIBS) and thus 
				dnl probably using the aix compiler.
				AC_MSG_WARN($AIX_COMPILE_INFO)
				;;
				*)
				P_UTHREAD_LIBS=error
				plib_save_LIBS="$LIBS"
				for thread_lib in "" pthread pthread32 pthreads thread; do
					if test x"$thread_lib" = x; then
						add_thread_lib=""
						IN=""
					else
						add_thread_lib="-l$thread_lib"
						IN=" in -l$thread_lib"
					fi
				
					LIBS="$add_thread_lib $plib_save_LIBS"
			
					AC_MSG_CHECKING(for thread create/join$IN)
					if test x"$have_threads" = xposix; then
						AC_TRY_RUN(AC_LANG_SOURCE([plib_pthread_test]),
							   plib_result=yes,
							   plib_result=no,
							   [AC_LINK_IFELSE([AC_LANG_SOURCE([plib_pthread_test])],
									   plib_result=yes,
									   plib_result=no)])
					else
						AC_TRY_RUN(AC_LANG_SOURCE([plib_thread_test]),
							   plib_result=yes,
							   plib_result=no,
							   [AC_LINK_IFELSE([AC_LANG_SOURCE([plib_thread_test])],
									   plib_result=yes,
									   plib_result=no)])
					fi
					AC_MSG_RESULT($plib_result)
			
					if test "$plib_result" = "yes" ; then
						P_UTHREAD_LIBS="$add_thread_lib"
						break
					fi
				done
				
				if test "x$P_UTHREAD_LIBS" = xerror; then
					AC_MSG_ERROR($THREAD_LIBS_NOT_FOUND_1$have_threads$THREAD_LIBS_NOT_FOUND_2)
				fi 
				
				LIBS="$plib_save_LIBS"
				;;
			esac
		fi

		plib_save_LIBS="$LIBS"
		for thread_lib in "" rt rte posix4; do
			if test x"$thread_lib" = x; then
				add_thread_lib=""
				IN=""
			else
				add_thread_lib="-l$thread_lib"
				IN=" in -l$thread_lib"
			fi
			LIBS="$add_thread_lib $plib_save_LIBS"
	
			AC_MSG_CHECKING(for sched_yield$IN)
			AC_TRY_RUN(AC_LANG_SOURCE([plib_sched_test]), plib_result=yes, plib_result=no,
				   [AC_LINK_IFELSE([AC_LANG_SOURCE([plib_sched_test])],
						   plib_result=yes,
						   plib_result=no)])
			AC_MSG_RESULT($plib_result)

			if test "$plib_result" = "yes" ; then	    
				P_UTHREAD_LIBS="$P_UTHREAD_LIBS $add_thread_lib"
				break
			fi
		done

		LIBS="$plib_save_LIBS"
		CPPFLAGS="$plib_save_CPPFLAGS"
		;;
		win32)
		;;
		*)
		P_UTHREAD_LIBS=error
		;;
	esac

	if test "x$P_UTHREAD_LIBS" = xerror; then
		AC_MSG_ERROR($THREAD_LIBS_NOT_FOUND_1$have_threads$THREAD_LIBS_NOT_FOUND_2)  
	else
		AC_MSG_CHECKING(thread related libraries)
		AC_MSG_RESULT($P_UTHREAD_LIBS)
	fi
fi

dnl Hack to deal with:
dnl
dnl  a) GCC < 3.3 for Linux doesn't include -lpthread when
dnl     building shared libraries with linux.
dnl  b) FreeBSD doesn't do this either.
case $host in
	*-*-freebsd*|*-*-linux*)
	P_LIBS_FOR_UTHREAD="`echo $P_UTHREAD_LIBS | sed s/-pthread/-lpthread/`"
	;;
	*) 	 
	P_LIBS_FOR_UTHREAD="$P_UTHREAD_LIBS" 	 
	;;
esac

dnl PLib without threads can be used in MT-apps
if test -z $P_UTHREAD_CFLAGS; then
	P_UTHREAD_CFLAGS="-D_REENTRANT -D_PTHREADS -D_THREAD_SAFE -D_POSIX4_DRAFT_SOURCE -D_POSIX4A_DRAFT10_SOURCE -U_OSF_SOURCE"
fi

dnl Try to setup static mutex
mutex_has_default=no
if test "x$have_threads" = xposix; then
	mutex_has_default=yes
	mutex_default_type='pthread_mutex_t'
	mutex_default_init='PTHREAD_MUTEX_INITIALIZER'
	mutex_header_file='pthread.h'
elif test "x$have_threads" = xsolaris; then
	mutex_has_default=yes
	mutex_default_type='mutex_t'
	mutex_default_init='DEFAULTMUTEX'
	mutex_header_file='thread.h'
fi

if test $mutex_has_default = yes ; then
	plib_save_CPPFLAGS="$CPPFLAGS"
	plib_save_LIBS="$LIBS"
	LIBS="$P_LIBS_FOR_UTHREAD $LIBS"
	CPPFLAGS="$CPPFLAGS $PUTHREAD_COMPILE_IMPL_DEFINES"
	PLIB_SIZEOF([#include <$mutex_header_file>],
		    $mutex_default_type,
		    pmutex)
	PLIB_BYTE_CONTENTS([#include <$mutex_header_file>],
			   $mutex_default_type,
			   pmutex,
			   $plib_cv_sizeof_pmutex,
			   $mutex_default_init)
	if test x"$plib_cv_byte_contents_pmutex" = xno; then
		mutex_has_default=no
	fi
	CPPFLAGS="$plib_save_CPPFLAGS"
	LIBS="$plib_save_LIBS"
fi

AM_CONDITIONAL(HAVE_THREADS, [test "x$have_threads" = xposix] || [test "x$have_threads" = xsolaris] || \
			     [test "x$have_threads" = xwin32])
AM_CONDITIONAL(HAVE_POSIX_THREADS, [test "x$have_threads" = xposix])
AM_CONDITIONAL(HAVE_SOLARIS_THREADS, [test "x$have_threads" = xsolaris])
AM_CONDITIONAL(HAVE_WIN32_THREADS, [test "x$have_threads" = xwin32])
AC_SUBST(P_UTHREAD_CFLAGS)
AC_SUBST(P_LIBS_FOR_UTHREAD)
AC_SUBST(PUTHREAD_COMPILE_IMPL_DEFINES)

dnl Test programs for basic POSIX IPC functionality
m4_define([plib_posix_ipc_test],
	  [#include <stdlib.h> 
	   #include <fcntl.h>
	   #include <semaphore.h>
	   #include <sys/stat.h>
	   #include <sys/mman.h>
	   int main()
	   {
		sem_t *sem = sem_open ("/plib_ac_test_semaphore", O_CREAT, 0660, 1);
		if (sem != NULL) {
			sem_close (sem);
			sem_unlink ("/plib_ac_test_semaphore");
		}
		shm_open ("/plib_ac_test_shm", 0, 0660);
		return 0;
	   }
])

IPC_LIBS_NOT_FOUND_1="Can't find the libraries for the POSIX IPC
		implementation of "

IPC_LIBS_NOT_FOUND_2=". Please choose another IPC implementation or
		provide information on your IPC implementation.
		You can also run 'configure --disable-ipc' 
		to compile without IPC support."

dnl Determination of IPC implementation
	P_IPC_LIBS=
	P_IPC_IMPLEMENTATION=

if test "x$want_ipc" = xsysv || test "x$want_ipc" = xposix; then
	have_ipc="$want_ipc";
else
	have_ipc=no
fi

if test "x$want_ipc" = xyes || test "x$want_ipc" = xwin32; then
       	case $host in
               	*-*-mingw*)
		have_ipc=win32
		;;
	esac
fi

if test "x$want_ipc" = xyes || test "x$want_ipc" = xsysv \
			    || test "x$want_ipc" = xposix \
			    || text "x$want_ipc" = xwin32; then
	if test "x$have_ipc" = xno; then
		AC_TRY_LINK([#include <stdlib.h>
			     #include <sys/types.h>
			     #include <sys/ipc.h>
			     #include <sys/sem.h>
			     #include <sys/shm.h>],
			     [int shm;
			      semget (1234, 1, 0);
			      shm = shmget (1234, 0, 0);
			      if (shm != -1)
			      		shmctl (shm, IPC_RMID, NULL);],
			     [have_ipc=sysv
			      P_IPC_IMPLEMENTATION=sysv])
	fi
	if test "x$have_ipc" = xno; then
		AC_COMPILE_IFELSE([AC_LANG_SOURCE([plib_posix_ipc_test])],
				  have_ipc=posix)
	fi

	AC_MSG_CHECKING(for IPC implementation)

	if test x"$have_ipc" = xno; then
		AC_MSG_RESULT(none available)
		AC_MSG_WARN([No implementation of IPC found!])
	else
		AC_MSG_RESULT($have_ipc)
	fi

	dnl Determination of P_IPC_LIBS
	if test "x$have_ipc" = xposix; then
		if test x"$P_IPC_LIBS" = x; then
			P_IPC_LIBS=error
			plib_save_LIBS="$LIBS"
			for ipc_lib in "" rt rte posix4; do
				if test x"$ipc_lib" = x; then
					add_ipc_lib=""
					IN=""
				else
					add_ipc_lib="-l$ipc_lib"
					IN=" in -l$ipc_lib"
				fi
			
				LIBS="$add_ipc_lib $plib_save_LIBS"
				AC_MSG_CHECKING(for POSIX IPC$IN)
				
				AC_TRY_RUN(AC_LANG_SOURCE([plib_posix_ipc_test]),
					   plib_result=yes,
					   plib_result=no,
					   [AC_LINK_IFELSE([AC_LANG_SOURCE([plib_posix_ipc_test])],
							   plib_result=yes,
							   plib_result=no)])

				AC_MSG_RESULT($plib_result)
			
				if test "$plib_result" = "yes" ; then
					P_IPC_LIBS="$add_thread_lib"
					P_IPC_IMPLEMENTATION=posix
					break
				fi
			done

			if test "x$P_IPC_LIBS" = xerror; then
				AC_MSG_ERROR($IPC_LIBS_NOT_FOUND_1$have_ipc$IPC_LIBS_NOT_FOUND_2)
			fi 
			LIBS="$plib_save_LIBS"
		fi
	fi
	
	AC_MSG_CHECKING(IPC related libraries)
	AC_MSG_RESULT($P_IPC_LIBS)
fi

AM_CONDITIONAL(HAVE_IPC, [test "x$have_ipc" = xsysv] || [test "x$have_ipc" = xposix] || \
			 [test "x$have_ipc" = xwin32])
AM_CONDITIONAL(HAVE_SYSV_IPC, [test "x$have_ipc" = xsysv])
AM_CONDITIONAL(HAVE_POSIX_IPC, [test "x$have_ipc" = xposix])
AM_CONDITIONAL(HAVE_WIN32_IPC, [test "x$have_ipc" = xwin32])
AC_SUBST(P_IPC_IMPLEMENTATION)
AC_SUBST(P_IPC_LIBS)

dnl Check for socket calls libs
AC_SEARCH_LIBS([socket], [socket])
if test "x$ac_cv_search_socket" != xno && test "x$ac_cv_search_socket" != x"none required"; then
	P_SOCKET_LIBS="$ac_cv_search_socket"
fi
AC_SUBST(P_SOCKET_LIBS)

case $host in
	*-*-mingw*)
		P_LIBS_EXTRA="-lws2_32 -lole32"
    		;;
  	*)
		P_LIBS_EXTRA=""
   		;;
esac
AC_SUBST(P_LIBS_EXTRA)

dnl Doxygen documentation
AC_PATH_PROG(doxygen_prog, doxygen, no)
AC_MSG_CHECKING([whether to build Doxygen documentation])

if test x$doxygen_prog = xno; then
	have_doxygen=no
else
	have_doxygen=yes
fi

if test x$enable_doxygen_docs = xauto; then
	if test x$have_doxygen = xno; then
		enable_doxygen_docs=no
	else
		enable_doxygen_docs=yes
	fi
fi

if test x$enable_doxygen_docs = xyes; then
	if test x$have_doxygen = xno; then
		AC_MSG_ERROR([Building Doxygen docs explicitly required, but Doxygen not found.])
	fi
fi

AM_CONDITIONAL(DOXYGEN_DOCS_ENABLED, test x$enable_doxygen_docs = xyes)
AC_SUBST(DOXYGEN_PROG)
AC_MSG_RESULT($enable_doxygen_docs)

AC_CONFIG_COMMANDS([plibconfig.h],
[
	outfile=plibconfig.h-tmp
	cat > $outfile <<\_______EOF
/* plibconfig.h
 *
 * This is a generated file.  Please modify 'configure.ac'
 */

#ifndef __PLIBCONFIG_H__
#define __PLIBCONFIG_H__

#include <pmacros.h>
_______EOF
	if test x$plib_limits_h = xyes; then
		echo '#include <limits.h>' >> $outfile
	fi
	if test x$plib_float_h = xyes; then
		echo '#include <float.h>' >> $outfile
	fi
	if test x$plib_values_h = xyes; then
		echo '#include <values.h>' >> $outfile
	fi

cat >>$outfile <<_______EOF

P_BEGIN_DECLS

#define P_MINFLOAT	$plib_mf
#define P_MAXFLOAT	$plib_Mf
#define P_MINDOUBLE	$plib_md
#define P_MAXDOUBLE	$plib_Md
#define P_MINSHORT	$plib_ms
#define P_MAXSHORT	$plib_Ms
#define P_MAXUSHORT	$plib_Mus
#define P_MININT	$plib_mi
#define P_MAXINT	$plib_Mi
#define P_MAXUINT	$plib_Mui
#define P_MINLONG	$plib_ml
#define P_MAXLONG	$plib_Ml
#define P_MAXULONG	$plib_Mul

_______EOF

	dnl This should always be true in a modern C/C++ compiler
	cat >>$outfile <<_______EOF
typedef signed char pint8;
typedef unsigned char puint8;
_______EOF


	if test -n "$pint16"; then
	  cat >>$outfile <<_______EOF
typedef signed $pint16	pint16;
typedef unsigned $pint16 puint16;
#define PINT16_MODIFIER		$pint16_modifier
#define PINT16_FORMAT		$pint16_format
#define PUINT16_FORMAT		$puint16_format

_______EOF
	fi


	if test -n "$pint32"; then
	  cat >>$outfile <<_______EOF
typedef signed $pint32 pint32;
typedef unsigned $pint32 puint32;
#define PINT32_MODIFIER		$pint32_modifier
#define PINT32_FORMAT		$pint32_format
#define PUINT32_FORMAT		$puint32_format
_______EOF
	fi

	cat >>$outfile <<_______EOF
typedef signed $pint64 pint64;
typedef unsigned $pint64 puint64;
_______EOF


	if test x$pint64_format != x ; then
	  cat >>$outfile <<_______EOF
#define PINT64_MODIFIER		$pint64_modifier
#define PINT64_FORMAT		$pint64_format
#define PUINT64_FORMAT		$puint64_format
_______EOF
        else
	  cat >>$outfile <<_______EOF
#undef PINT64_MODIFIER
#undef PINT64_FORMAT
#undef PUINT64_FORMAT
_______EOF
        fi           


        cat >>$outfile <<_______EOF
typedef signed $plib_size_type_define pssize;
typedef unsigned $plib_size_type_define psize;
#define PSIZE_MODIFIER		$psize_modifier
#define PSSIZE_FORMAT		$pssize_format
#define PSIZE_FORMAT		$psize_format

typedef pint64 poffset;
#define POFFSET_MODIFIER	PINT64_MODIFIER
#define POFFSET_FORMAT		PINT64_FORMAT

#define P_MAXSIZE		P_MAXU$plib_msize_type
#define P_MINSSIZE		P_MIN$plib_msize_type
#define P_MAXSSIZE		P_MAX$plib_msize_type

#define PLIB_SIZEOF_VOID_P	$plib_void_p
#define PLIB_SIZEOF_LONG	$plib_long
#define PLIB_SIZEOF_SIZE_T	$plib_size_t

_______EOF

	if test -z "$plib_unknown_void_p"; then
		cat >>$outfile <<_______EOF
#define PPOINTER_TO_INT(p)	((pint)  ${plib_ppi_cast} (p))
#define PPOINTER_TO_UINT(p)	((puint) ${plib_ppui_cast} (p))

#define PINT_TO_POINTER(i)	((ppointer) ${plib_ppi_cast} (i))
#define PUINT_TO_POINTER(u)	((ppointer) ${plib_ppui_cast} (u))

typedef signed $plib_intptr_type_define pintptr;
typedef unsigned $plib_intptr_type_define puintptr;

#define PINTPTR_MODIFIER      $pintptr_modifier
#define PINTPTR_FORMAT        $pintptr_format
#define PUINTPTR_FORMAT       $puintptr_format

_______EOF
	else
	  echo '#error SIZEOF_VOID_P unknown - This should never happen' >>$outfile
	fi
		
	echo >>$outfile
	if test x$p_mutex_has_default = xyes; then
		cat >>$outfile <<_______EOF
typedef struct _PStaticMutex
{
	union {
	char	pad[[$p_mutex_sizeof]];
	double	dummy_double;
	long	dummy_long;
	void	*dummy_pointer;
	} static_mutex;
} PStaticMutex;

#define	P_STATIC_MUTEX_INIT		{ { { $p_mutex_contents} } }
#define	p_static_mutex_get_mutex(mutex)	((PMutex*) ((ppointer) ((mutex)->static_mutex.pad)))
_______EOF
	else
		cat >>$outfile <<_______EOF
typedef struct _PMutex* PStaticMutex;
#define P_STATIC_MUTEX_INIT		NULL
extern P_LIB_API PStaticMutex p_static_mutex_get_mutex_impl (PStaticMutex *mutex);
#define p_static_mutex_get_mutex(mutex)	(p_static_mutex_get_mutex_impl (mutex))
_______EOF
	fi

	if test -n "$plib_socklen_type_define"; then
		cat >>$outfile <<_______EOF
typedef $plib_socklen_type_define socklen_t;

_______EOF
	fi

	if test "x$plib_have_sockaddr_storage" = xno; then
		cat >>$outfile <<_______EOF
#define __ss_aligntype		unsigned long int
#define _SS_SIZE		256
#define _SS_PADSIZE		(_SS_SIZE - (2 * sizeof (__ss_aligntype)))
struct sockaddr_storage {
	unsigned short int ss_family;
	__ss_aligntype __ss_align;
	char __ss_padding[[_SS_PADSIZE]];
};

_______EOF
	fi

	if test x"$p_memory_barrier_needed" != xno; then
		echo >>$outfile
		echo "#define P_ATOMIC_OP_MEMORY_BARRIER_NEEDED 1" >>$outfile
	fi

	p_bit_sizes="16 32 64"
	for bits in $p_bit_sizes; do
	  cat >>$outfile <<_______EOF
#define PINT${bits}_TO_${p_bs_native}(val)	((pint${bits}) (val))
#define PUINT${bits}_TO_${p_bs_native}(val)	((puint${bits}) (val))
#define PINT${bits}_TO_${p_bs_alien}(val)	((pint${bits}) PUINT${bits}_SWAP_BYTES (val))
#define PUINT${bits}_TO_${p_bs_alien}(val)	(PUINT${bits}_SWAP_BYTES (val))
_______EOF
	done

	cat >>$outfile <<_______EOF
#define PLONG_TO_LE(val)	((plong) PINT${plongbits}_TO_LE (val))
#define PULONG_TO_LE(val)	((pulong) PUINT${plongbits}_TO_LE (val))
#define PLONG_TO_BE(val)	((plong) PINT${plongbits}_TO_BE (val))
#define PULONG_TO_BE(val)	((pulong) PUINT${plongbits}_TO_BE (val))
#define PINT_TO_LE(val)		((pint) PINT${pintbits}_TO_LE (val))
#define PUINT_TO_LE(val)	((puint) PUINT${pintbits}_TO_LE (val))
#define PINT_TO_BE(val)		((pint) PINT${pintbits}_TO_BE (val))
#define PUINT_TO_BE(val)	((puint) PUINT${pintbits}_TO_BE (val))
#define PSIZE_TO_LE(val)	((psize) PUINT${psizebits}_TO_LE (val))
#define PSSIZE_TO_LE(val)	((pssize) PINT${psizebits}_TO_LE (val))
#define PSIZE_TO_BE(val)	((psize) PUINT${psizebits}_TO_BE (val))
#define PSSIZE_TO_BE(val)	((pssize) PINT${psizebits}_TO_BE (val))
#define P_BYTE_ORDER		$p_byte_order

P_END_DECLS

#endif /* __PLIBCONFIG_H__ */
_______EOF

	if cmp -s $outfile plibconfig.h; then
	  AC_MSG_NOTICE([plibconfig.h is unchanged])
	  rm -f $outfile
	else
	  mv $outfile plibconfig.h
	fi
],[


case 2 in
$ac_cv_sizeof_short)		
	pint16=short
	gint16_modifier='"h"'
	pint16_format='"hi"'
	puint16_format='"hu"'
	;;
$ac_cv_sizeof_int)		
	pint16=int
	pint16_modifier='""'
	pint16_format='"i"'
	pint16_format='"u"'
	;;
esac

case 4 in
$ac_cv_sizeof_short)		
	pint32=short
	pint32_modifier='"h"'
	pint32_format='"hi"'
	puint32_format='"hu"'
	;;
$ac_cv_sizeof_int)		
	pint32=int
	pint32_modifier='""'
	pint32_format='"i"'
	puint32_format='"u"'
	;;
$ac_cv_sizeof_long)		
	pint32=long
	pint32_modifier='"l"'
	pint32_format='"li"'
	puint32_format='"lu"'
	;;
esac

case 8 in
$ac_cv_sizeof_int)
	pint64=int
	pint64_modifier='""'
	pint64_format='"i"'
	puint64_format='"u"'
	;;
$ac_cv_sizeof_long)
	pint64=long
	pint64_modifier='"l"'
	pint64_format='"li"'
	puint64_format='"lu"'
	;;
$ac_cv_sizeof_long_long)
	pint64='long long'
	if test -n "$plib_cv_long_long_format"; then
		pint64_modifier='"'$plib_cv_long_long_format'"'
		pint64_format='"'$plib_cv_long_long_format'i"'
		puint64_format='"'$plib_cv_long_long_format'u"'
	fi
	;;
$ac_cv_sizeof___int64)
	pint64='__int64'
	if test -n "$plib_cv_long_long_format"; then
		pint64_modifier='"'$plib_cv_long_long_format'"'
		pint64_format='"'$plib_cv_long_long_format'i"'
		puint64_format='"'$plib_cv_long_long_format'u"'
	fi
	;;
esac

plib_size_type_define="$plib_size_type"

case "$plib_size_type" in
short)
	psize_modifier='"h"'
	psize_format='"hu"'
	pssize_format='"hi"'
	plib_msize_type='SHRT'
	;;
int)
	psize_modifier='""'
	psize_format='"u"'
	pssize_format='"i"'
	plib_msize_type='INT'
	;;
long)
	psize_modifier='"l"'
	psize_format='"lu"'
	pssize_format='"li"'
	plib_msize_type='LONG'
	;;
"long long"|__int64)
	psize_modifier='"I64"'
	psize_format='"I64u"'
	pssize_format='"I64i"'
	plib_msize_type='INT64'
  ;;
esac

plib_socklen_type_define="$plib_socklen_type"
plib_have_sockaddr_storage="$have_sockaddr_storage"

plib_size_t=$ac_cv_sizeof_size_t
plib_void_p=$ac_cv_sizeof_void_p
plib_long=$ac_cv_sizeof_long

pintbits=`expr $ac_cv_sizeof_int \* 8`
plongbits=`expr $ac_cv_sizeof_long \* 8`
psizebits=`expr $ac_cv_sizeof_size_t \* 8`

p_mutex_has_default="$mutex_has_default"
p_mutex_sizeof="$plib_cv_sizeof_pmutex"
p_mutex_contents="$plib_cv_byte_contents_pmutex"

p_memory_barrier_needed="$plib_memory_barrier_needed"

case $ac_cv_sizeof_void_p in
$ac_cv_sizeof_int)
	plib_intptr_type_define=int
	pintptr_modifier='""'
	pintptr_format='"i"'
	puintptr_format='"u"'
	;;
$ac_cv_sizeof_long)
	plib_intptr_type_define=long
	pintptr_modifier='"l"'
	pintptr_format='"li"'
	puintptr_format='"lu"'
	;;
$ac_cv_sizeof_long_long)
	plib_intptr_type_define='long long'
	pintptr_modifier='"I64"'
	pintptr_format='"I64i"'
	puintptr_format='"I64u"'
	;;
$ac_cv_sizeof___int64)
	plib_intptr_type_define=__int64
	pintptr_modifier='"I64"'
	pintptr_format='"I64i"'
	puintptr_format='"I64u"'
	;;
*)
	plib_unknown_void_p=yes
	;;
esac

case xyes in
x$ac_cv_header_float_h)
	plib_float_h=yes
	plib_mf=FLT_MIN
	plib_Mf=FLT_MAX
	plib_md=DBL_MIN
	plib_Md=DBL_MAX
	;;
x$ac_cv_header_values_h)
	plib_values_h=yes
	plib_mf=MINFLOAT
	plib_Mf=MAXFLOAT
	plib_md=MINDOUBLE
	plib_Md=MAXDOUBLE
	;;
esac

case xyes in
x$ac_cv_header_limits_h)
	plib_limits_h=yes
	plib_ms=SHRT_MIN
	plib_Ms=SHRT_MAX
	plib_Mus=USHRT_MAX
	plib_mi=INT_MIN
	plib_Mi=INT_MAX
	plib_Mui=UINT_MAX
	plib_ml=LONG_MIN
	plib_Ml=LONG_MAX
	plib_Mul=ULONG_MAX
	;;
x$ac_cv_header_values_h)
	plib_values_h=yes
	plib_ms=MINSHORT
	plib_Ms=MAXSHORT
	plib_Mus="(((pushort) P_MAXSHORT) * 2 + 1)"
	plib_mi=MININT
	plib_Mi=MAXINT
	plib_Mui="(((puint) P_MAXINT) * 2 + 1)"
	plib_ml=MINLONG
	plib_Ml=MAXLONG
	plib_Mul="(((pulong) P_MAXLONG) * 2 + 1)"
	;;
esac

if test x$ac_cv_c_bigendian = xyes; then
	p_byte_order=P_BIG_ENDIAN
	p_bs_native=BE
	p_bs_alien=LE
else
	p_byte_order=P_LITTLE_ENDIAN
	p_bs_native=LE
	p_bs_alien=BE
fi
])

AC_CONFIG_FILES([Makefile src/Makefile src/plib.def plib.pc plib-uninstalled.pc Doxyfile])
AC_OUTPUT

