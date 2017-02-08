function (plibsys_scosv_print_threading_message)
        message ("
                 You need a working port of FSU Pthreads in order to
                 compile with multi-threading support. Please refer to
                 http://moss.csc.ncsu.edu/~mueller/pthreads/ for more
                 details. Make sure that it is compiled with thread-safe
                 memory allocation (usually -DMALLOC macro definition).
                 ")
endfunction (plibsys_scosv_print_threading_message)
