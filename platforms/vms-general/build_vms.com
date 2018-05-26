$!
$! Copyright 2011, Richard Levitte <richard@levitte.org>
$! Copyright 2014, John Malmberg <wb8tyw@qsl.net>
$! Copyright 2016-2018, Alexander Saprykin <saprykin.spb@gmail.com>
$!
$! Permission to use, copy, modify, and/or distribute this software for any
$! purpose with or without fee is hereby granted, provided that the above
$! copyright notice and this permission notice appear in all copies.
$!
$! THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
$! WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
$! MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
$! ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
$! WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
$! ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
$! OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
$!
$!===========================================================================
$! Command-line options:
$!
$!    32            Compile with 32-bit pointers.
$!    BIGENDIAN     Compile for a big endian host.
$!    CCQUAL=x      Add "x" to the C compiler qualifiers.
$!    DEBUG         Build in debug mode.
$!    CLEAN         Only perform clean after the previous build.
$!    TESTS=(x)     Build library tests. Comma separated test names or leave
$!                  empty to build all the tests.
$!                  Example 1 (curtain tests): TESTS=(pmem,puthread)
$!                  Example 2 (all tests): TESTS
$!    RUN_TESTS     Runs all tests.
$!    NOLIB         Skip library buidling. Useful when you want to rebuild
$!                  particular tests.
$!===========================================================================
$!
$!
$! Save the original default dev:[dir], and arrange for its restoration
$! at exit.
$!---------------------------------------------------------------------
$ orig_def = f$environment("DEFAULT")
$ on error then goto common_exit
$ on severe_error then goto common_exit
$ on control_y then goto common_exit
$!
$ ctrl_y       = 1556
$ proc         = f$environment("PROCEDURE")
$ proc_fid     = f$file_attributes(proc, "FID")
$ proc_dev     = f$parse(proc, , , "DEVICE")
$ proc_dir     = f$parse(proc, , , "DIRECTORY")
$ proc_name    = f$parse(proc, , , "NAME")
$ proc_type    = f$parse(proc, , , "TYPE")
$ proc_dev_dir = proc_dev + proc_dir
$!
$! Have to manually parse the device for a search list.
$! Can not use the f$parse() as it will return the first name
$! in the search list.
$!
$ orig_def_dev = f$element(0, ":", orig_def) + ":"
$ if orig_def_dev .eqs. "::" then orig_def_dev = "sys$disk:"
$ test_proc = orig_def_dev + proc_dir + proc_name + proc_type
$!
$! If we can find this file using the default directory
$! then we know that we should use the original device from the
$! default directory which could be a search list.
$!
$ test_proc_fid = f$file_attributes(test_proc, "FID")
$!
$ if (test_proc_fid .eq. proc_fid)
$ then
$     proc_dev_dir = orig_def_dev + proc_dir
$ endif
$!
$! Verbose output message stuff. Define symbol to "write sys$output".
$! vo_c - verbose output for compile
$!
$ vo_c := "write sys$output"
$!
$! Determine the main distribution directory ("[--]") in an
$! ODS5-tolerant (case-insensitive) way.  (We do assume that the only
$! "]" or ">" is the one at the end.)
$!
$! Some non-US VMS installations report ">" for the directory delimiter
$! so do not assume that it is "]".
$!
$ orig_def_len = f$length(orig_def)
$ delim = f$extract(orig_def_len - 1, 1, orig_def)
$!
$ set default 'proc_dev_dir'
$ set default [--.src]
$ base_src_dir = f$environment("default")
$ set default 'proc_dev_dir'
$!
$! Define the architecture-specific destination directory name
$! -----------------------------------------------------------
$!
$ if (f$getsyi("HW_MODEL") .lt. 1024)
$ then
$      'vo_c' "%PLIBSYS-F-NOTSUP, VAX platform is not supported, sorry :("
$      goto common_exit
$ else
$      arch_name = ""
$      arch_name = arch_name + f$edit(f$getsyi("ARCH_NAME"), "UPCASE")
$!
$      if (arch_name .eqs. "") then arch_name = "UNK"
$!
$      node_swvers   = f$getsyi("node_swvers")
$      version_patch = f$extract(1, f$length(node_swvers), node_swvers)
$      maj_ver       = f$element(0, ".", version_patch)
$      min_ver_patch = f$element(1, ".", version_patch)
$      min_ver       = f$element(0, "-", min_ver_patch)
$      patch         = f$element(1, "-", min_ver_patch)
$!
$      if maj_ver .lts. "8" .or. min_ver .lts. "4"
$      then
$          'vo_c' "%PLIBSYS-F-NOTSUP, only OpenVMS 8.4 and above are supported, sorry :("
$          goto common_exit
$      endif
$ endif
$!
$ objdir = proc_dev_dir - delim + ".''arch_name'" + delim
$!
$! Parse input arguments
$! ---------------------
$! Allow arguments to be grouped together with comma or separated by spaces
$! Do no know if we will need more than 8.
$ args = "," + p1 + "," + p2 + "," + p3 + "," + p4 + ","
$ args = args + p5 + "," + p6 + "," + p7 + "," + p8 + ","
$!
$! Provide lower case version to simplify parsing.
$ args_lower = f$edit(args, "LOWERCASE,COLLAPSE")
$!
$ args_len = f$length(args)
$ args_lower_len = f$length(args_lower)
$!
$ if f$locate(",clean,", args_lower) .lt. args_lower_len
$ then
$     'vo_c' "Cleaning up previous build..."
$     set default 'proc_dev_dir'
$!
$     if f$search("''arch_name'.DIR") .nes. ""
$     then
$         set prot=w:d []'arch_name'.DIR;*
$         delete/tree [.'arch_name'...]*.*;*
$         delete []'arch_name'.DIR;*
$     endif
$!
$     goto common_exit
$ endif
$!
$ build_64   = 1
$ if f$locate(",32,", args_lower) .lt. args_lower_len
$ then
$     build_64 = 0
$ endif
$!
$ big_endian = 0
$ if f$locate(",bigendian,", args_lower) .lt. args_lower_len
$ then
$     big_endian = 1
$ endif
$!
$ cc_extra = ""
$ args_loc = f$locate(",ccqual=", args_lower)
$ if args_loc .lt. args_lower_len
$ then
$     arg = f$extract(args_loc + 1, args_lower_len, args_lower)
$     arg_val = f$element(0, ",", arg)
$     cc_extra = f$element(1, "=", arg_val)
$ endif
$!
$ is_debug = 0
$ if f$locate(",debug,", args_lower) .lt. args_lower_len
$ then
$     is_debug = 1
$ endif
$!
$ is_tests = 0
$ test_list = ""
$ if f$locate(",tests,", args_lower) .lt. args_lower_len
$ then
$     is_tests = 1
$ else
$     args_loc = f$locate(",tests=(", args_lower)
$     if args_loc .lt. args_lower_len
$     then
$         is_tests          = 1
$         arg               = f$extract(args_loc + 1, args_lower_len, args_lower)
$         arg_val           = f$element(0, ")", arg)
$         test_list_val     = f$element(1, "=", arg_val) - "(" - ")"
$         test_list_val     = f$edit(test_list_val, "COLLAPSE")
$         test_list_counter = 0
$
$ test_list_loop: 
$         next_test_val = f$element (test_list_counter, ",", test_list_val)
$         if next_test_val .nes. "" .and. next_test_val .nes. ","
$         then
$             test_list         = test_list + next_test_val + " "
$             test_list_counter = test_list_counter + 1
$             goto test_list_loop
$         endif
$     endif
$ endif
$!
$ run_tests = 0
$ if f$locate(",run_tests,", args_lower) .lt. args_lower_len
$ then
$     run_tests = 1
$ endif
$!
$! Prepare build directory
$! -----------------------
$!
$! When building on a search list, need to do a create to make sure that
$! the output directory exists, since the clean procedure tries to delete
$! it.
$!
$ if f$search("''proc_dev_dir'''arch_name'.DIR") .eqs. ""
$ then
$     create/dir 'objdir'/prot=o:rwed
$ endif
$!
$ set default 'objdir'
$ if f$search("CXX_REPOSITORY.DIR") .nes. ""
$ then
$     set prot=w:d []CXX_REPOSITORY.DIR;*
$     delete/tree [.CXX_REPOSITORY...]*.*;*
$     delete []CXX_REPOSITORY.DIR;*
$ endif
$!
$ if f$locate(",nolib,", args_lower) .lt. args_lower_len
$ then
$     goto build_tests
$ endif
$!
$! Generate platform-specific config file
$! --------------------------------------
$!
$ if f$search("plibsysconfig.h") .nes. "" then delete plibsysconfig.h;*
$!
$! Get the version number
$! ----------------------
$!
$ i = 0
$ open/read/error=version_loop_end vhf [---]CMakeLists.txt
$ version_loop:
$     read/end=version_loop_end vhf line_in
$!
$     if line_in .eqs. "" then goto version_loop
$!
$     if f$locate("set (PLIBSYS_VERSION_MAJOR ", line_in) .eq. 0
$     then
$         plibsys_vmajor = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (PLIBSYS_VERSION_MINOR ", line_in) .eq. 0
$     then
$         plibsys_vminor = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (PLIBSYS_VERSION_PATCH ", line_in) .eq. 0
$     then
$         plibsys_vpatch = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (PLIBSYS_VERSION_NUM ", line_in) .eq. 0
$     then
$         plibsys_vnum = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if i .lt 4 then goto version_loop
$ version_loop_end:
$ close vhf
$!
$! Write config file
$! -----------------
$!
$ open/write/error=config_write_end chf plibsysconfig.h
$ write chf "#ifndef PLIBSYS_HEADER_PLIBSYSCONFIG_H"
$ write chf "#define PLIBSYS_HEADER_PLIBSYSCONFIG_H"
$ write chf ""
$ write chf "#define PLIBSYS_VERSION_MAJOR ''plibsys_vmajor'"
$ write chf "#define PLIBSYS_VERSION_MINOR ''plibsys_vminor'"
$ write chf "#define PLIBSYS_VERSION_PATCH ''plibsys_vpatch'"
$ write chf "#define PLIBSYS_VERSION_STR ""''plibsys_vmajor'.''plibsys_vminor'.''plibsys_vpatch'"""
$ write chf "#define PLIBSYS_VERSION ''plibsys_vnum'"
$ write chf ""
$ write chf "#define PLIBSYS_SIZEOF_SAFAMILY_T 1"
$ write chf ""
$ write chf "#include <pmacros.h>"
$ write chf ""
$ write chf "#include <float.h>"
$ write chf "#include <limits.h>"
$ write chf ""
$ write chf "P_BEGIN_DECLS"
$ write chf ""
$ write chf "#define P_MINFLOAT    FLT_MIN"
$ write chf "#define P_MAXFLOAT    FLT_MAX"
$ write chf "#define P_MINDOUBLE   DBL_MIN"
$ write chf "#define P_MAXDOUBLE   DBL_MAX"
$ write chf "#define P_MINSHORT    SHRT_MIN"
$ write chf "#define P_MAXSHORT    SHRT_MAX"
$ write chf "#define P_MAXUSHORT   USHRT_MAX"
$ write chf "#define P_MININT      INT_MIN"
$ write chf "#define P_MAXINT      INT_MAX"
$ write chf "#define P_MAXUINT     UINT_MAX"
$ write chf "#define P_MINLONG     LONG_MIN"
$ write chf "#define P_MAXLONG     LONG_MAX"
$ write chf "#define P_MAXULONG    ULONG_MAX"
$ write chf ""
$ write chf "#define PLIBSYS_MMAP_HAS_MAP_ANONYMOUS"
$ write chf "#define PLIBSYS_HAS_NANOSLEEP"
$ write chf "#define PLIBSYS_HAS_GETADDRINFO"
$ write chf "#define PLIBSYS_HAS_POSIX_SCHEDULING"
$ write chf "#define PLIBSYS_HAS_POSIX_STACKSIZE"
$ write chf "#define PLIBSYS_HAS_SOCKADDR_STORAGE"
$ write chf "#define PLIBSYS_SOCKADDR_HAS_SA_LEN"
$ write chf "#define PLIBSYS_SOCKADDR_IN6_HAS_SCOPEID"
$ write chf "#define PLIBSYS_SOCKADDR_IN6_HAS_FLOWINFO"
$ write chf ""
$!
$ if build_64 .eqs. "1"
$ then
$     write chf "#define PLIBSYS_SIZEOF_VOID_P 8"
$     write chf "#define PLIBSYS_SIZEOF_SIZE_T 8"
$ else
$     write chf "#define PLIBSYS_SIZEOF_VOID_P 4"
$     write chf "#define PLIBSYS_SIZEOF_SIZE_T 4"
$ endif
$!
$ write chf "#define PLIBSYS_SIZEOF_LONG 4"
$ write chf ""
$!
$ if big_endian .eqs. "1"
$ then
$     write chf "#define P_BYTE_ORDER P_BIG_ENDIAN"
$ else
$     write chf "#define P_BYTE_ORDER P_LITTLE_ENDIAN"
$ endif
$!
$ write chf ""
$ write chf "P_END_DECLS"
$ write chf ""
$ write chf "#endif /* PLIBSYS_HEADER_PLIBSYSCONFIG_H */"
$ config_write_end:
$     close chf
$!
$! Prepare sources for compilation
$! -------------------------------
$!
$ cc_link_params = ""
$ cc_params = "/NAMES=(AS_IS,SHORTENED)"
$ cc_params = cc_params + "/DEFINE=(PLIBSYS_COMPILATION,_REENTRANT,_POSIX_EXIT)"
$ cc_params = cc_params + "/INCLUDE_DIRECTORY=(''objdir',''base_src_dir')"
$ cc_params = cc_params + "/FLOAT=IEEE/IEEE_MODE=DENORM_RESULTS"
$!
$ if build_64 .eqs. "1"
$ then
$     cc_params = cc_params + "/POINTER_SIZE=64"
$ else
$     cc_params = cc_params + "/POINTER_SIZE=32"
$ endif
$!
$ if cc_extra .nes. ""
$ then
$     cc_params = cc_params + " " + cc_extra
$ endif
$!
$ if is_debug .eqs. "1"
$ then
$     cc_params = cc_params + "/DEBUG/NOOPTIMIZE/LIST/SHOW=ALL"
$     cc_link_params = "/DEBUG/TRACEBACK"
$ else
$     cc_link_params = "/NODEBUG/NOTRACEBACK"
$ endif
$!
$ plibsys_src = "patomic-decc.c pcondvariable-posix.c pcryptohash-gost3411.c pcryptohash-md5.c"
$ plibsys_src = plibsys_src + " pcryptohash-sha1.c pcryptohash-sha2-256.c pcryptohash-sha2-512.c"
$ plibsys_src = plibsys_src + " pcryptohash-sha3.c pcryptohash.c pdir-posix.c pdir.c"
$ plibsys_src = plibsys_src + " perror.c pfile.c phashtable.c pinifile.c pipc.c plibraryloader-posix.c"
$ plibsys_src = plibsys_src + " plist.c pmain.c pmem.c pmutex-posix.c pprocess.c prwlock-posix.c"
$ plibsys_src = plibsys_src + " psemaphore-posix.c pshm-posix.c pshmbuffer.c psocket.c"
$ plibsys_src = plibsys_src + " psocketaddress.c pspinlock-decc.c pstring.c psysclose-unix.c"
$ plibsys_src = plibsys_src + " ptimeprofiler-posix.c ptimeprofiler.c ptree-avl.c ptree-bst.c"
$ plibsys_src = plibsys_src + " ptree-rb.c ptree.c puthread-posix.c puthread.c"
$!
$! Inform about building
$! ---------------------
$!
$ if build_64 .eqs. "1"
$ then
$     'vo_c' "Building for ''arch_name' (64-bit)"
$ else
$     'vo_c' "Building for ''arch_name' (32-bit)"
$ endif
$!
$! Compile library modules
$! -----------------------
$!
$ 'vo_c' "Compiling object modules..."
$ src_counter = 0
$ plibsys_src = f$edit(plibsys_src, "COMPRESS")
$ plibsys_objs = ""
$!
$ src_loop:
$     next_src = f$element (src_counter, " ", plibsys_src)
$     if next_src .nes. "" .and. next_src .nes. " "
$     then
$         'vo_c' "[CC] ''next_src'"
$         cc [---.src]'next_src' 'cc_params'
$!
$         src_counter = src_counter + 1
$!
$         obj_file = f$extract (0, f$length (next_src) - 1, next_src) + "obj"
$         plibsys_objs = plibsys_objs + "''obj_file',"
$         purge 'obj_file'
$!
$         goto src_loop
$     endif
$!
$ plibsys_objs = f$extract (0, f$length (plibsys_objs) - 1, plibsys_objs)
$!
$! Create library
$! --------------
$!
$ 'vo_c' "Creating object library..."
$ library/CREATE/INSERT/REPLACE /LIST=PLIBSYS.LIS PLIBSYS.OLB 'plibsys_objs'
$ library/COMPRESS PLIBSYS.OLB
$ purge PLIBSYS.OLB
$ purge PLIBSYS.LIS
$!
$ 'vo_c' "Creating shared library..."
$ link/SHAREABLE=PLIBSYS.EXE /MAP=PLIBSYS.MAP 'cc_link_params' 'plibsys_objs', [-]plibsys.opt/OPTION
$ purge PLIBSYS.EXE
$ purge PLIBSYS.MAP
$!
$! Testing area
$! ------------
$!
$ build_tests:
$ test_list_full = "patomic pcondvariable pcryptohash pdir"
$ test_list_full = test_list_full + " perror pfile phashtable pinifile plibraryloader plist"
$ test_list_full = test_list_full + " pmacros pmain pmem pmutex pprocess prwlock psemaphore"
$ test_list_full = test_list_full + " pshm pshmbuffer psocket psocketaddress pspinlock pstdarg"
$ test_list_full = test_list_full + " pstring ptimeprofiler ptree ptypes puthread"
$!
$ if is_tests .eqs. "0"
$ then
$     goto build_done
$ endif
$!
$! Write link options file
$! -----------------------
$!
$ if f$search("plibsys_link.opt") .nes. "" then delete plibsys_link.opt;*
$!
$ open/write/error=link_write_end lhf plibsys_link.opt
$ write lhf "''objdir'PLIBSYS.EXE/SHARE"
$ write lhf ""
$ link_write_end:
$     close lhf
$!
$! Compile tests
$! -------------------------
$!
$ if test_list .nes. ""
$ then
$     plibsys_tests = f$edit(test_list, "TRIM")
$ else
$     plibsys_tests = test_list_full
$ endif
$!
$ 'vo_c' "Compiling test executables..."
$ test_counter = 0
$ plibsys_tests = f$edit(plibsys_tests, "COMPRESS")
$!
$ cxx_params = "/INCLUDE=(''objdir',''base_src_dir')"
$ cxx_params = cxx_params + "/DEFINE=(__USE_STD_IOSTREAM)/NAMES=(AS_IS, SHORTENED)"
$ cxx_params = cxx_params + "/FLOAT=IEEE/IEEE_MODE=DENORM_RESULTS"
$!
$ if build_64 .eqs. "1"
$ then
$     set noon
$     define/user/nolog sys$output NL:
$     define/user/nolog sys$error NL:
$     cxx/POINTER_SIZE=64=ARGV NL:
$!
$     if ($STATUS .and. %X0FFF0000) .eq. %X00030000
$     then
$!
$!        If we got here, it means DCL complained like this:
$!        %DCL-W-NOVALU, value not allowed - remove value specification
$!        \64=\
$!
$!        If the compiler was run, logicals defined in /USER would
$!        have been deassigned automatically.  However, when DCL
$!        complains, they aren't, so we do it here (it might be
$!        unnecessary, but just in case there will be another error
$!        message further on that we don't want to miss).
$!
$         deassign/user/nolog sys$error
$         deassign/user/nolog sys$output
$         cxx_params = cxx_params + "/POINTER_SIZE=64"
$     else
$         cxx_params = cxx_params + "/POINTER_SIZE=64=ARGV"
$     endif
$ else
$     cxx_params = cxx_params + "/POINTER_SIZE=32"
$ endif
$!
$ if is_debug .eqs. "1"
$ then
$     cxx_params = cxx_params + "/DEBUG/NOOPTIMIZE/LIST/SHOW=ALL"
$ endif
$!
$ test_loop:
$     next_test = f$element (test_counter, " ", plibsys_tests)
$     if next_test .nes. "" .and. next_test .nes. " "
$     then
$         next_test = next_test + "_test"
$         'vo_c' "[CXX    ] ''next_test'.cpp"
$         cxx [---.tests]'next_test'.cpp 'cxx_params'
$!
$         'vo_c' "[CXXLINK] ''next_test'.obj"
$          cxxlink 'next_test'.obj,'objdir'plibsys_link.opt/OPTION /THREADS_ENABLE
$!
$         if f$search("CXX_REPOSITORY.DIR") .nes. ""
$         then
$             set prot=w:d []CXX_REPOSITORY.DIR;*
$             delete/tree [.CXX_REPOSITORY...]*.*;*
$             delete []CXX_REPOSITORY.DIR;*
$         endif
$!
$         purge 'next_test'.obj
$         purge 'next_test'.exe
$!
$         test_counter = test_counter + 1
$         goto test_loop
$     endif
$!
$ build_done:
$     'vo_c' "Build done."
$!
$! Run unit tests
$! --------------
$!
$ if run_tests .eqs. "0"
$ then
$     if is_tests .eqs. "1"
$     then
$         'vo_c' "To run tests invoke: @build_vms.com NOLIB RUN_TESTS"
$     endif
$     goto common_exit
$ endif
$!
$ 'vo_c' "Running tests..."
$ test_counter = 0
$ tests_passed = 0
$!
$ run_loop:
$     next_test = f$element (test_counter, " ", test_list_full)
$     if next_test .nes. "" .and. next_test .nes. " "
$     then
$         if f$search("''next_test'_test.exe") .eqs. ""
$         then
$             'vo_c' "[SKIP] Test not found: ''next_test'"
$             goto run_loop_next
$         endif
$!
$         'vo_c' "[RUN ] ''next_test'"
$!
$         define/user/nolog sys$error NL:
$         define/user/nolog sys$output NL:
$         define/user/nolog plibsys 'objdir'PLIBSYS.EXE
$         define/user/nolog test_imgdir 'objdir'
$!
$!        Disable error cheking for the test binary
$!
$         set noon
$!
$         xrun := $test_imgdir:'next_test'_test.exe
$         if next_test .eqs. "plibraryloader"
$         then
$             xrun 'objdir'PLIBSYS.EXE
$         else
$             xrun
$         endif
$!
$         if $STATUS .eqs. "%X00000001"
$         then
$             'vo_c' "[PASS] Test passed: ''next_test'"
$             tests_passed = tests_passed + 1
$         else
$             'vo_c' "[FAIL] *** Test failed: ''next_test'"
$         endif
$!
$         set on
$!
$ run_loop_next:
$         test_counter = test_counter + 1
$         goto run_loop
$     endif
$!
$ 'vo_c' "Tests passed: ''tests_passed'/''test_counter'"
$!
$! In case of error during the last test
$ deassign/user/nolog sys$error
$ deassign/user/nolog sys$output
$ deassign/user/nolog plibsys
$ deassign/user/nolog test_imgdir
$!
$ common_exit:
$     set default 'orig_def'
$     exit
