# The MIT License
#
# Copyright (C) 2017 Jean-Damien Durand <jeandamiendurand@gmail.com>
# Copyright (C) 2019 Alexander Saprykin <saprykin.spb@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# 'Software'), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
                     #include <stdlib.h>
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

                     int main () {
                        f (0, 42);
                        exit (0);
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
