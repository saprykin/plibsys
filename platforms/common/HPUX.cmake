include (CheckFunctionExists)

function (plibsys_hpux_detect_libraryloader_model result)
        message (STATUS "Checking whether dlopen() presents")

        check_function_exists (dlopen PLIBSYS_HPUX_HAS_DLOPEN)

        if (PLIBSYS_HPUX_HAS_DLOPEN)
                message (STATUS "Checking whether dlopen() presents - yes")
                set (${result} posix PARENT_SCOPE)
        else()
                message (STATUS "Checking whether dlopen() presents - no")
                set (${result} shl PARENT_SCOPE)
        endif()
endfunction (plibsys_hpux_detect_libraryloader_model)
