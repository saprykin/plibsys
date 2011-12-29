## Portability defines that help interoperate with classic and modern autoconfs
ifdef([AC_TR_SH],[
define([PLIB_TR_SH],[AC_TR_SH([$1])])
define([PLIB_TR_CPP],[AC_TR_CPP([$1])])
], [
define([PLIB_TR_SH],
       [patsubst(translit([[$1]], [*+], [pp]), [[^a-zA-Z0-9_]], [_])])
define([PLIB_TR_CPP],
       [patsubst(translit([[$1]],
  	                  [*abcdefghijklmnopqrstuvwxyz],
 			  [PABCDEFGHIJKLMNOPQRSTUVWXYZ]),
		 [[^A-Z0-9_]], [_])])
])

# PLIB_CHECK_COMPILE_WARNINGS(PROGRAM, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------------------------------------------------------------
# Try to compile PROGRAM, check for warnings
m4_define([PLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([AC_LANG_SOURCE([$1])])])dnl
rm -f conftest.$ac_objext
plib_ac_compile_save="$ac_compile"
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext'
AS_IF([_AC_EVAL_STDERR($ac_compile) &&
         AC_TRY_COMMAND([(if test -s conftest.err; then false ; else true; fi)])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])
ac_compile="$plib_ac_compile_save"
rm -f conftest.$ac_objext conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# PLIB_CHECK_COMPILE_WARNINGS
# PLIB_SIZEOF (INCLUDES, TYPE, ALIAS)
# ---------------------------------------------------------------
# The definition here is based of that of AC_CHECK_SIZEOF
AC_DEFUN([PLIB_SIZEOF],
[AS_LITERAL_IF([$3], [],
               [AC_FATAL([$0: requires literal arguments])])dnl
AC_CACHE_CHECK([size of $2], AS_TR_SH([plib_cv_sizeof_$3]),
[ # The cast to unsigned long works around a bug in the HP C Compiler
  # version HP92453-01 B.11.11.23709.GP, which incorrectly rejects
  # declarations like `int a3[[(sizeof (unsigned char)) >= 0]];'.
  # This bug is HP SR number 8606223364.
  _AC_COMPUTE_INT([(long) (sizeof ($2))],
                  [AS_TR_SH([plib_cv_sizeof_$3])],
                  [AC_INCLUDES_DEFAULT([$1])],
                  [AC_MSG_ERROR([cannot compute sizeof ($2), 77])])
])dnl
AC_DEFINE_UNQUOTED(PLIB_TR_CPP(plib_sizeof_$3), $AS_TR_SH([plib_cv_sizeof_$3]),
                   [The size of $3, as computed by sizeof.])
])# PLIB_SIZEOF

dnl PLIB_BYTE_CONTENTS (INCLUDES, TYPE, ALIAS, N_BYTES, INITIALIZER)
AC_DEFUN([PLIB_BYTE_CONTENTS],
[pushdef([plib_ByteContents], PLIB_TR_SH([plib_cv_byte_contents_$3]))dnl
AC_CACHE_CHECK([byte contents of $5], plib_ByteContents,
[AC_TRY_RUN([#include <stdio.h>
$1
main()
{
  static $2 tv = $5;
  char *p = (char*) &tv;
  int i;
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  for (i = 0; i < $4; i++)
    fprintf(f, "%s%d", i?",":"", *(p++));
  fprintf(f, "\n");
  exit(0);
}], 
   [plib_ByteContents=`cat conftestval`  dnl''
], 
   [plib_ByteContents=no],
   [plib_ByteContents=no])])
AC_DEFINE_UNQUOTED(PLIB_TR_CPP(plib_byte_contents_$3), [$[]plib_ByteContents],
	[Byte contents of $3])
popdef([plib_ByteContents])dnl
])

