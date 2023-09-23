# The MIT License
#
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

function (plibsys_detect_thread_name has_pthread_np_h result)
        set (PTHREAD_HEADERS "#include <pthread.h>")

        if (${has_pthread_np_h})
                set (PTHREAD_HEADERS "${PTHREAD_HEADERS}\n#include<pthread_np.h>")
        endif()

        # Check pthread_setname_np() with 1 arg

        if (NOT PLIBSYS_THREAD_SETTER)
                check_c_source_compiles ("
                     ${PTHREAD_HEADERS}

                     int main () {
                        pthread_setname_np (\"thread_name\");
                        return 0;
                     }"
                    PLIBSYS_HAS_POSIX_SETNAME_NP_1
                )

                if (PLIBSYS_HAS_POSIX_SETNAME_NP_1)
                        set (PLIBSYS_THREAD_SETTER "pthread_setname_np")
                endif()
        endif()

        # Check pthread_setname_np() with 2 args

        if (NOT PLIBSYS_THREAD_SETTER)
                check_c_source_compiles ("
                     ${PTHREAD_HEADERS}

                     int main () {
                        pthread_setname_np ((pthread_t) 0, \"thread_name\");
                        return 0;
                     }"
                    PLIBSYS_HAS_POSIX_SETNAME_NP_2
                )

                if (PLIBSYS_HAS_POSIX_SETNAME_NP_2)
                        set (PLIBSYS_THREAD_SETTER "pthread_setname_np")
                endif()
        endif()

        # Check pthread_setname_np() with 3 args

        if (NOT PLIBSYS_THREAD_SETTER)
                check_c_source_compiles ("
                     ${PTHREAD_HEADERS}

                     int main () {
                        pthread_setname_np ((pthread_t) 0, \"thread_name\", 0);
                        return 0;
                     }"
                    PLIBSYS_HAS_POSIX_SETNAME_NP_3
                )

                if (PLIBSYS_HAS_POSIX_SETNAME_NP_3)
                        set (PLIBSYS_THREAD_SETTER "pthread_setname_np")
                endif()
        endif()

        # Check pthread_set_name_np()

        if (NOT PLIBSYS_THREAD_SETTER)
                check_c_source_compiles ("
                     ${PTHREAD_HEADERS}

                     int main () {
                        pthread_set_name_np ((pthread_t) 0, \"thread_name\");
                        return 0;
                     }"
                    PLIBSYS_HAS_POSIX_SET_NAME_NP
                )

                if (PLIBSYS_HAS_POSIX_SET_NAME_NP)
                        set (PLIBSYS_THREAD_SETTER "pthread_set_name_np")
                endif()
        endif()

        # The last try is prctl()

        if (NOT PLIBSYS_THREAD_SETTER)
                check_c_source_compiles ("
                     #include <sys/prctl.h>
                     #include <linux/prctl.h>

                     int main () {
                        prctl (PR_SET_NAME, \"thread_name\", NULL, NULL, NULL);
                        return 0;
                     }"
                    PLIBSYS_PRCTL
                )

                if (PLIBSYS_PRCTL)
                        set (PLIBSYS_THREAD_SETTER "prctl")
                endif()
        endif()

        # It seems that CMake (old versions like 2.8.x - 2.10.x) has a bug,
        # such that when passing empty values to the parent scope, these variable
        # are not treated as empty, thereby use NONE value instead

        if (PLIBSYS_THREAD_SETTER STREQUAL "pthread_setname_np")
                set (${result} "PLIBSYS_HAS_PTHREAD_SETNAME" PARENT_SCOPE)
        elseif (PLIBSYS_THREAD_SETTER STREQUAL "pthread_set_name_np")
                set (${result} "PLIBSYS_HAS_PTHREAD_SET_NAME" PARENT_SCOPE)
        elseif (PLIBSYS_THREAD_SETTER STREQUAL "prctl")
                set (${result} "PLIBSYS_HAS_PTHREAD_PRCTL" PARENT_SCOPE)
        else()
                set (${result} "NONE" PARENT_SCOPE)
        endif()
endfunction (plibsys_detect_thread_name)
