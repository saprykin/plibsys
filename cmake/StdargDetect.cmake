function (plibsys_detect_va_copy result)
    #
    # On platforms that supports va_start() and al.
    # there is sometimes the need to do va_copy() when
    # calling another va_list aware function.
    #
    # This means that va_copy() is not needed (thus not available)
    # everywhere.
    #
    # We depend on stdarg.h in any case, stdio.h in some cases.
    #
    # Known implementations vary from va_copy to __va_copy (old proposed name).
    # We check the _va_copy eventually.
    #
    set (P_VA_COPY FALSE)
    foreach (KEYWORD "va_copy" "_va_copy" "__va_copy")
        check_c_source_compiles ("
                     #include <stdio.h>
                     #include <stdarg.h>

                     void f (int i, ...) {
                        va_list args1, args2;

                        va_start (args1, i);
                        ${KEYWORD}(args2, args1);
                        if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42) {
                            exit (1);
                        }
                        va_end (args1);
                        va_end (args2);
                     }

                     int main() {
                        f (0, 42);
                        exit(0);
                     }"
                    PLIBSYS_${KEYWORD}
        )

        if (PLIBSYS_${KEYWORD})
            set (P_VA_COPY ${KEYWORD})
            break()
        endif()
    endforeach()

    # Assign result
    set (${result} ${P_VA_COPY} PARENT_SCOPE)

endfunction (plibsys_detect_va_copy)
