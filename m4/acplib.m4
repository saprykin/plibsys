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

