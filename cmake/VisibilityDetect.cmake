function (plibsys_detect_visibility cflags ldflags)
        if (WIN32 OR CYGWIN OR MSYS)
            set (${cflags} "" PARENT_SCOPE)
            set (${ldflags} "" PARENT_SCOPE)
            return()
        endif()

        # Check GCC
        check_c_source_compiles ("int main () {
                                        #if (__GNUC__ >= 4) && \\\\
                                            !defined(__sun)  && !defined(sun)  && \\\\
                                            !defined(__hpux) && !defined(hpux) && \\\\
                                            !defined(_AIX) && !defined(__CYGWIN__) && !defined(__MSYS__)
                                        return 0;
                                        #else
                                        stop_compile_here
                                        #endif
                                 }"
                                 PLIBSYS_HAS_GCC_VISIBILITY
        )

        if (PLIBSYS_HAS_GCC_VISIBILITY)
                set (${cflags} "-fvisibility=hidden" PARENT_SCOPE)
                set (${ldflags} "" PARENT_SCOPE)
                return()
        endif()

        # Check Sun Studio
        check_c_source_compiles ("int main () {
                                        #if (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)) || \\\\
                                            (defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x5110))
                                        return 0;
                                        #else
                                        stop_compile_here
                                        #endif
                                 }"
                                 PLIBSYS_HAS_SUN_VISIBILITY
        )

        if (PLIBSYS_HAS_SUN_VISIBILITY)
                set (${cflags} "" PARENT_SCOPE)
                set (${ldlags} "-xldscope=__hidden" PARENT_SCOPE)
                return()
        endif()

        # Check IBM XL C
        check_c_source_compiles ("int main () {
                                        #if (defined(__xlC__) && (__xlC__ >= 0x0D01))
                                        return 0;
                                        #else
                                        stop_compile_here
                                        #endif
                                 }"
                                 PLIBSYS_HAS_XLC_VISIBILITY
        )

        if (PLIBSYS_HAS_XLC_VISIBILITY)
                set (${cflags} "-qvisibility=hidden" PARENT_SCOPE)
                set (${ldflags} "" PARENT_SCOPE)
                return()
        endif()

        # Check HP C/aC++
        check_c_source_compiles ("int main () {
                                        #if (defined(__HP_cc)  && (__HP_cc >= 0x061500)) || \\\\
                                            (defined(__HP_aCC) && (__HP_aCC >= 0x061500))
                                        return 0;
                                        #else
                                        stop_compile_here
                                        #endif
                                 }"
                                 PLIBSYS_HAS_HP_VISIBILITY
        )

        if (PLIBSYS_HAS_HP_VISIBILITY)
                set (${cflags} "-Bhidden" PARENT_SCOPE)
                set (${ldflags} "" PARENT_SCOPE)
                return()
        endif()

        # Check Clang
        check_c_source_compiles ("int main () {
                                        #if defined(__has_attribute) && __has_attribute(visibility)
                                        return 0;
                                        #else
                                        stop_compile_here
                                        #endif
                                 }"
                                 PLIBSYS_HAS_CLANG_VISIBILITY
        )

        if (PLIBSYS_HAS_CLANG_VISIBILITY)
                set (${cflags} "-fvisibility=hidden" PARENT_SCOPE)
                set (${ldflags} "" PARENT_SCOPE)
                return()
        endif()

        # Empty result
        set (${cflags} "" PARENT_SCOPE)
        set (${ldflags} "" PARENT_SCOPE)
endfunction (plibsys_detect_visibility)
